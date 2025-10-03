# Known Issues and Solutions

## Training Freezes at Step 5000

### Symptom
Training stops and freezes exactly at step 4999 or after approximately 5000 steps.

### Root Cause
The `EvalCallback` in Stable-Baselines3 was configured to run evaluation every 5000 steps (`eval_freq=5_000`). When evaluation started, it tried to:

1. Create a second game instance
2. Use the same port (5555) as the training environment
3. This caused a port conflict
4. The evaluation environment would hang trying to connect
5. Training would freeze waiting for evaluation to complete

### Solution
The evaluation callback is now **disabled by default**.

**Default behavior (recommended):**
```bash
# Evaluation callback disabled - no freezing
uv run python scripts/train.py --timesteps 1000000
```

**To enable evaluation (advanced):**
```bash
# Uses port 5556 for evaluation to avoid conflicts
uv run python scripts/train.py --timesteps 1000000 --enable-eval
```

When `--enable-eval` is used:
- Training environment uses port 5555
- Evaluation environment uses port 5556
- Two game instances run simultaneously
- May be slower but provides evaluation metrics

### Alternative: Monitor Progress with Tensorboard

Instead of using the evaluation callback, monitor training with Tensorboard:

```bash
# Terminal 1: Train
cd agent
uv run python scripts/train.py

# Terminal 2: Monitor
cd agent
uv run tensorboard --logdir logs
# Open http://localhost:6006
```

Tensorboard shows:
- Episode rewards
- Episode lengths
- Loss curves
- Policy metrics
- All in real-time!

### Alternative: Periodic Manual Evaluation

Train for a while, then evaluate separately:

```bash
# Train for 100k steps
uv run python scripts/train.py --timesteps 100000

# Evaluate the best model
uv run python scripts/evaluate.py models/best_model.zip --episodes 10

# Continue training
uv run python scripts/train.py --timesteps 200000

# Evaluate again
uv run python scripts/evaluate.py models/best_model.zip --episodes 10
```

This gives you full control and avoids any port conflicts!

## Other Port Conflicts

### Multiple Training Sessions

If you try to run multiple training sessions simultaneously, you'll get port conflicts.

**Solution: Use different ports**

```bash
# Terminal 1
uv run python -c "
from agent.envs.starship_env import StarshipEnv
from agent.scripts.train import train
env = StarshipEnv(port=5555)
train(total_timesteps=100000)
"

# Terminal 2 (use port 5556)
uv run python -c "
from agent.envs.starship_env import StarshipEnv
from agent.scripts.train import train
env = StarshipEnv(port=5556)
train(total_timesteps=100000)
"
```

Or create separate training scripts with different ports.

## Process Not Cleaning Up

### Symptom
Port still in use after training stops, can't restart training.

### Solution
Kill the game process manually:

```bash
# macOS/Linux
lsof -ti:5555 | xargs kill -9

# Or kill all starship_game processes
pkill -9 starship_game

# Windows
netstat -ano | findstr :5555
taskkill /PID <pid> /F
```

## Game Window Not Closing

### Symptom
When using `--render`, game window stays open after training stops.

### Solution
The environment should close the window automatically, but if it doesn't:

```bash
# Force kill game processes
pkill -9 starship_game  # macOS/Linux
```

Or press Ctrl+C to interrupt training, which should trigger cleanup.

## Slow Training with Render Mode

### Symptom
Training is very slow when using `--render` flag.

### Cause
Rendering the game window adds significant overhead (~30-50% slower).

### Solution
Only use `--render` for:
- Initial debugging
- Short test runs
- Watching agent behavior
- Demos

For actual training, use headless mode (default):
```bash
uv run python scripts/train.py --timesteps 1000000
```

Much faster! Monitor progress with Tensorboard instead.

## Memory Usage Growing

### Symptom
Memory usage increases over long training sessions.

### Cause
Each episode restart creates a new game process. Old processes should be cleaned up, but sometimes they linger.

### Solution

**Monitor processes:**
```bash
# Check how many game processes are running
ps aux | grep starship_game

# Should see only 1-2 processes
```

**If you see many processes:**
```bash
# Kill all old game processes
pkill starship_game

# Restart training
uv run python scripts/train.py
```

**For very long training sessions:**
- Consider restarting training periodically
- Models are saved every 10,000 steps
- You can resume from the latest checkpoint

## Environment Not Resetting

### Symptom
Game seems stuck in game over state, doesn't reset between episodes.

### Cause
The environment is configured to always restart the game process on reset, which should prevent this. But if it happens:

### Solution

**Check if reset is being called:**
```python
obs, info = env.reset()  # Should terminate old process and start new one
```

**If issue persists:**
1. Check game logs for errors
2. Verify process is actually restarting (check PIDs)
3. Try the test script:
```bash
uv run python scripts/test_reset.py
```

## Can't Connect to Game

### Symptom
```
ConnectionRefusedError: [Errno 61] Connection refused
```

### Solutions

**1. Check if port is available:**
```bash
lsof -i :5555  # Should show nothing or game process
```

**2. Check if game is built:**
```bash
ls -la build/starship_game
```

**3. Run connection test:**
```bash
cd agent
uv run python scripts/test_connection.py
```

**4. Try manual start:**
```bash
# Terminal 1: Start game manually
./build/starship_game --rl-mode --headless

# Terminal 2: Run training
cd agent
uv run python scripts/train.py
```

## Python Dependencies Issues

### Symptom
```
ModuleNotFoundError: No module named 'gymnasium'
```

### Solution
```bash
cd agent
rm -rf .venv uv.lock
uv sync
```

## Getting Help

If you're still stuck:

1. **Run diagnostic tests:**
```bash
cd agent
uv run python scripts/test_connection.py
uv run python scripts/test_reset.py
```

2. **Check logs:**
- Game logs appear in terminal where game runs
- Python errors appear in training terminal
- Tensorboard logs in `agent/logs/`

3. **Enable debug logging:**
```bash
# For game
SDL_LOG_PRIORITY=debug ./build/starship_game --rl-mode

# For Python
python scripts/train.py  # Errors will print
```

4. **Try minimal example:**
```python
from agent.envs.starship_env import StarshipEnv

env = StarshipEnv()
obs, _ = env.reset()
print(f"Reset OK: {obs.shape}")

for i in range(10):
    obs, reward, done, truncated, _ = env.step(env.action_space.sample())
    print(f"Step {i}: reward={reward}, done={done}")

env.close()
print("Test passed!")
```