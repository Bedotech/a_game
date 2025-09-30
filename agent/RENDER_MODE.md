# Render Mode Guide

This guide explains the difference between **headless** and **render** modes during training.

## TL;DR

```bash
# Fast training (no window) - Recommended for long training sessions
./scripts/quick_start.sh

# Visual training (see the game) - Great for debugging and watching progress
./scripts/quick_start.sh --render
```

## Headless Mode (Default)

**What it does:**
- Runs the game without creating a visible window
- Game still runs and renders internally, just not displayed
- Faster training due to less overhead

**When to use:**
- Long training sessions (1M+ timesteps)
- Running on a server or remote machine
- When you want maximum training speed
- Overnight/background training

**Example:**
```bash
cd agent
./scripts/quick_start.sh
# or
uv run python scripts/train.py
```

**Output:**
```
🎮 Render mode: DISABLED (headless training for speed)
   💡 Use './quick_start.sh --render' to see the game

🚀 Starting training with the game environment...
🎮 Starting game in headless mode: /path/to/starship_game --rl-mode --port=5555 --headless
✅ Connected to game on port 5555

Starting training...
---------------------------------
| rollout/           |          |
|    ep_len_mean     | 42.5     |
|    ep_rew_mean     | 38.2     |
| time/              |          |
|    fps             | 1247     |  ← Fast!
```

## Render Mode

**What it does:**
- Opens a game window showing what the agent sees
- You can watch the agent play in real-time
- Slightly slower due to rendering overhead

**When to use:**
- Initial testing and debugging
- Watching agent behavior and learning
- Demos and presentations
- Verifying the agent is actually playing
- Short training sessions to see progress

**Example:**
```bash
cd agent
./scripts/quick_start.sh --render
# or
uv run python scripts/train.py --render
```

**Output:**
```
🎮 Render mode: ENABLED (you will see the game window)

🚀 Starting training with the game environment...
🎮 Starting game in render mode: /path/to/starship_game --rl-mode --port=5555
✅ Connected to game on port 5555

[Game window opens showing the starship and asteroids]

Starting training...
---------------------------------
| rollout/           |          |
|    ep_len_mean     | 42.5     |
|    ep_rew_mean     | 38.2     |
| time/              |          |
|    fps             | 845      |  ← Slower due to rendering
```

## Performance Comparison

| Mode | FPS (approx) | Time for 1M steps | Use Case |
|------|--------------|-------------------|----------|
| Headless | 1000-1500 | ~10-15 min | Long training |
| Render | 500-1000 | ~15-30 min | Debugging, demos |

**Note:** Actual performance depends on your hardware and game complexity.

## Visual Example

### Headless Mode
```
Terminal Only:
┌─────────────────────────────────┐
│ $ ./quick_start.sh              │
│                                 │
│ 🎮 Render mode: DISABLED        │
│ 🚀 Starting training...         │
│                                 │
│ Episode 1: reward=42.5          │
│ Episode 2: reward=51.2          │
│ Episode 3: reward=38.9          │
│ ...                             │
└─────────────────────────────────┘
```

### Render Mode
```
Terminal + Game Window:
┌─────────────────────────────────┐  ┌────────────────────────┐
│ $ ./quick_start.sh --render     │  │   Starship Game        │
│                                 │  │                        │
│ 🎮 Render mode: ENABLED         │  │    🚀                  │
│ 🚀 Starting training...         │  │                        │
│                                 │  │         ☄️             │
│ Episode 1: reward=42.5          │  │                   ☄️   │
│ Episode 2: reward=51.2          │  │      ☄️                │
│ Episode 3: reward=38.9          │  │                        │
│ ...                             │  └────────────────────────┘
└─────────────────────────────────┘
    You see this happening live! →
```

## Switching Modes

### During Training

You **cannot** switch modes during training. You need to:
1. Stop training (Ctrl+C)
2. Restart with desired mode

### For Different Sessions

```bash
# Session 1: Quick test with rendering (1000 episodes)
uv run python scripts/train.py --render --timesteps 10000

# Session 2: Long training without rendering
uv run python scripts/train.py --timesteps 5000000

# Session 3: Resume with rendering to see progress
uv run python scripts/train.py --render --timesteps 100000
```

Models are saved to the same directory, so you can switch modes between sessions.

## Monitoring Without Render Mode

Even in headless mode, you can monitor training progress:

### 1. Tensorboard (Recommended)

```bash
# Start tensorboard in another terminal
cd agent
uv run tensorboard --logdir logs

# Open browser to http://localhost:6006
```

Tensorboard shows:
- Episode rewards over time
- Episode lengths
- Loss curves
- Value estimates
- Policy entropy
- And more!

### 2. Console Output

The training script prints progress:
```
Episode 10: reward=45.2, length=38
Episode 20: reward=58.9, length=52
Episode 30: reward=42.1, length=35
...
```

### 3. Evaluation

After training, evaluate with rendering:
```bash
uv run python scripts/evaluate.py models/best_model.zip --episodes 5
```

This will show the game window for the best trained model!

## Tips

### For Fast Training
- Use headless mode
- Close other applications
- Reduce `n_steps` and `batch_size` if memory is limited
- Use GPU if available (PyTorch CUDA)

### For Debugging
- Use render mode with fewer timesteps
- Start with `--timesteps 10000` to quickly see behavior
- Watch for obvious issues (stuck in corners, etc.)
- Then switch to headless for long training

### Best Practice
```bash
# 1. Quick test with rendering (2 minutes)
uv run python scripts/train.py --render --timesteps 50000

# 2. If it looks good, long training headless (overnight)
uv run python scripts/train.py --timesteps 5000000

# 3. Evaluate best model with rendering
uv run python scripts/evaluate.py models/best_model.zip
```

## Troubleshooting

### "No window appears in render mode"

**Possible causes:**
- Running on SSH/remote server without X11 forwarding
- macOS: Game needs screen recording permissions
- Headless system (no display server)

**Solutions:**
```bash
# Check if game can start with window
../build/starship_game --rl-mode

# On macOS: Grant screen recording permission in System Preferences
# On Linux: Ensure DISPLAY environment variable is set
echo $DISPLAY

# On remote: Use headless mode instead
./scripts/quick_start.sh
```

### "Render mode is very slow"

**Solutions:**
- Normal! Rendering adds overhead
- Use headless mode for training
- Use render mode only for evaluation
- Close other applications
- Check GPU usage

### "Want to see training but headless is too fast"

**Solution:**
Use Tensorboard! It's the best of both worlds:
```bash
# Terminal 1: Train headless
uv run python scripts/train.py

# Terminal 2: Watch with Tensorboard
uv run tensorboard --logdir logs
```

Then watch graphs update in real-time at http://localhost:6006

## Summary

| Feature | Headless | Render |
|---------|----------|--------|
| Speed | ⚡⚡⚡ Fast | ⚡⚡ Slower |
| Visual feedback | ❌ None | ✅ Live window |
| Good for training | ✅ Yes | ⚠️ Short sessions |
| Good for debugging | ⚠️ Limited | ✅ Yes |
| Good for demos | ❌ No | ✅ Yes |
| Remote compatible | ✅ Yes | ⚠️ Needs X11 |

**Recommendation:** Use headless for training, render for evaluation and debugging!