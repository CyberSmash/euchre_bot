# Neural Network Bot Roadmap

## Step 1: Observation Encoding (C++)

Create an `encode` function that converts an `Observation` into a flat `std::array<float, N>` suitable for neural network input. Two separate encodings, one per model:

**Play encoding** (~180 floats, exact size TBD):
- Hand: 24 floats (one per card, 1.0 if held)
- Trump suit: 4 floats (one-hot)
- Lead card: 25 floats (24 one-hot + 1 invalid flag for when leading)
- Trick cards: 4 x 25 floats (one-hot per player slot + invalid flag)
- Num played: 1 float (0-3 normalized, or 4 one-hot)
- Maker team: 1 float (0 or 1, relative to current player's team)
- Player position relative to dealer: 4 floats (one-hot)

**Bidding encoding** (~60 floats, exact size TBD):
- Hand: 24 floats (same as play)
- Face-up card: 25 floats (24 one-hot + 1 invalid flag)
- Phase: 4 floats (one-hot across BidRound1, BidRound2, GoAloneDecision, DealerPickupDiscard)
- Player position relative to dealer: 4 floats (one-hot)
- Maker team: 1 float (relevant for GoAlone and Discard phases)
- Trump suit: 4 floats (only set during GoAlone/Discard, zero during bidding rounds)

Put this in a header (e.g., `include/Encoding.hpp`) so both the data recorder and eventual NeuralBot use the same encoding. Define constants for the encoding sizes.

Validate by writing a few test cases: encode a known observation, check that the right floats are set.

## Step 2: Data Recorder (C++)

Create a `DataRecorder` class that captures training data during games. It sits on top of the engine and observes every decision.

**Record struct** (one per decision point):
- Encoded observation (the float array from step 1)
- Action taken (uint16_t)
- Result (float, filled in after game ends: 1.0 = win, 0.0 = loss)

**How it works:**
- Wraps or hooks into `Env::request_action` so every bot action is recorded
- Stores per-game trajectories in memory (a vector of Records)
- When the game ends, stamps every record in that game with the outcome
- Separates records into two buffers: bidding and play (for the two models)
- Periodically flushes buffers to disk as flat binary files

**File format:**
- Raw binary. Each record is the same fixed size.
- Header with metadata: encoding size, record count, format version
- Python reads with `numpy.fromfile()` or `numpy.memmap()`

**Performance considerations:**
- No disk I/O during gameplay, only in-memory appends
- Pre-allocate vectors to avoid reallocation
- Flush to disk between games, not during

## Step 3: Generate Training Data

Modify `main.cpp` (or create a separate tool) to run large batches of HeuristicBot games with the DataRecorder attached.

- Run HeuristicBot vs HeuristicBot for ~1M games
- Produces two binary files: one for bidding decisions, one for play decisions
- Log some stats: total records per file, win rate, average records per game
- Verify file sizes make sense (record size x count)

This is the dataset that the Python training script will consume.

## Step 4: Python Data Loading

Write a Python script or module that reads the binary training data into PyTorch.

- Read binary files with `numpy.fromfile()`, reshape into records
- Split into observation, action, and result arrays
- Create a PyTorch `Dataset` and `DataLoader`
- Split into train/validation sets
- Verify by spot-checking: decode a few observations back to human-readable form to make sure the encoding looks right

## Step 5: Model Architecture (Python/PyTorch)

Define two neural network models in PyTorch:

**Play model:**
- Input: play observation encoding (flat float array)
- Output: 24 logits (one per possible play card, actions 0-23)
- Architecture: start simple. A few fully connected layers with ReLU. Something like input -> 256 -> 128 -> 24. Can experiment with depth and width later.
- Loss: cross-entropy between predicted action probabilities and the actual action taken
- Masking: during inference, mask out illegal actions before picking. During training, the loss only needs to predict the correct action.

**Bidding model:**
- Input: bidding observation encoding (flat float array)
- Output: 8 logits (Pass, OrderUp, CallC, CallH, CallS, CallD, GoAloneYes, GoAloneNo — or however you want to slice the action space for bidding)
- Same architectural approach as play model, likely smaller

Start simple. A small network that trains fast is better for validating the pipeline. You can always make it bigger later.

## Step 6: Training Loop (Python)

Standard PyTorch supervised training loop for each model:

- Load data from step 4
- Forward pass: model(observation) -> action logits
- Compute loss: cross-entropy against the actual action
- Optionally weight the loss by result (so winning actions are reinforced more)
- Backward pass + optimizer step (Adam is a safe default)
- Track training and validation loss per epoch
- Save the best model checkpoint

Train for enough epochs that validation loss plateaus. Plot loss curves to verify the model is actually learning (loss should decrease).

**Sanity checks:**
- Does the model learn to pass most of the time in BidRound1? (HeuristicBot passes often)
- Does the play model learn to follow suit? (Always legal, so indirectly enforced)
- Is validation loss close to training loss? (If not, overfitting)

## Step 7: Export Model

Once training converges, export both models to a format C++ can load.

**ONNX path:**
- `torch.onnx.export(model, dummy_input, "play_model.onnx")`
- Verify with `onnxruntime` in Python first before moving to C++

**TorchScript path (alternative):**
- `torch.jit.script(model)` or `torch.jit.trace(model, dummy_input)`
- Save with `model.save("play_model.pt")`

Pick one format. ONNX is more portable and the runtime is lighter. TorchScript is more tightly integrated with PyTorch. Either works.

Test the exported model in Python against the original to make sure outputs match.

## Step 8: NeuralBot (C++)

Create a `NeuralBot` class that implements `IBot`, loading the exported models for inference.

**Dependencies:**
- If ONNX: add ONNX Runtime as a CMake dependency
- If TorchScript: add LibTorch as a CMake dependency

**Implementation:**
- Constructor loads both models (bidding + play) from file paths
- `play_trick`: encode observation (step 1), run play model, mask illegal actions from the output logits, pick the highest-probability legal action
- `bid_phase_1_action`, `bid_phase_2_action`, `go_alone_action`, `dealer_pickup_discard_action`: encode observation, run bidding model, mask illegal actions, pick best legal action
- The encoding function from step 1 is shared between the recorder and NeuralBot

**Action masking at inference time:**
- Model outputs raw logits for all possible actions
- Set logits for illegal actions to negative infinity
- Softmax the rest (or just argmax if you want deterministic play)
- Return the action with the highest score

## Step 9: Evaluate

Benchmark NeuralBot against the existing bots using the benchmark harness in main.cpp.

**Milestones to check:**
- NeuralBot vs RandomBot: should win handily (>90%) if it learned anything
- NeuralBot vs HeuristicBot: should be close to 50/50 if imitation learning worked well. It won't exceed HeuristicBot since it was trained on HeuristicBot data.
- NeuralBot full game completes without errors: basic sanity that action masking works and no illegal actions are produced
- Run a large batch (100k games) and check for any crashes or stalls

Add benchmark matchups to main.cpp and test cases to a test file.

## Step 10: Self-Play (Approach #2)

This is where the model moves beyond HeuristicBot. Switch from supervised imitation to reinforcement learning via self-play.

**What changes:**
- Instead of generating data with HeuristicBot, generate data with NeuralBot playing against itself
- Instead of training to imitate a specific action, train to maximize game outcome
- Loss function shifts from pure cross-entropy to a policy gradient method (e.g., REINFORCE) or a value-based method

**This requires nanobind bindings** to drive the engine from Python so the training loop can:
- Run a game step by step
- Query the model at each decision
- Collect the trajectory
- Update the model based on the outcome

**Considerations:**
- Start by having the current NeuralBot (from step 9) play against itself
- Periodically evaluate against HeuristicBot to track improvement
- The model should eventually surpass HeuristicBot's win rate
- This is the most experimental step — expect to iterate on reward shaping, learning rate, and architecture

## Dependencies Summary

- **C++ only (steps 1-3, 8-9):** ONNX Runtime or LibTorch (step 8 only)
- **Python (steps 4-7, 10):** PyTorch, NumPy
- **Python+C++ bridge (step 10):** nanobind
