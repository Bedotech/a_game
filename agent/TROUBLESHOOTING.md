# Troubleshooting Guide

Common issues and solutions for the Starship RL training setup.

## Connection Problems

### "Connection refused" Error

**Symptom:**
```
ConnectionRefusedError: [Errno 61] Connection refused
```

**Solution:**

1. **Test the connection:**
   ```bash
   cd agent
   uv run python scripts/test_connection.py
   ```

2. **Check if game is built:**
   ```bash
   ls -la ../build/starship_game
   ```
   If missing, build it:
   ```bash
   cd ..
   cmake -B build && cmake --build build
   ```

3. **Verify port is available:**
   ```bash
   # macOS/Linux
   lsof -i :5555

   # Windows
   netstat -an | findstr 5555
   ```

   If port is in use, either:
   - Kill the process using it
   - Use a different port: `StarshipEnv(port=5556)`

### Connection Times Out

**Symptom:**
```
Could not connect to game on port 5555 after 10 attempts
```

**Solution:**

1. **Start game manually first:**
   ```bash
   cd ..
   ./build/starship_game --rl-mode --headless
   ```
   You should see:
   ```
   Game bridge listening on port 5555
   Waiting for RL agent connection...
   ```

2. **Then run Python in another terminal:**
   ```bash
   cd agent
   uv run python scripts/train.py
   ```

3. **Check firewall settings:**
   - Ensure localhost (127.0.0.1) connections are allowed
   - Port 5555 should not be blocked

## Build Issues

### Game Bridge Not Found

**Symptom:**
```
fatal error: '../agent/utils/game_bridge.h' file not found
```

**Solution:**

1. **Verify file exists:**
   ```bash
   ls -la agent/utils/game_bridge.h
   ls -la agent/utils/game_bridge.c
   ```

2. **Clean and rebuild:**
   ```bash
   rm -rf build
   cmake -B build && cmake --build build
   ```

### Linking Errors on Windows

**Symptom:**
```
undefined reference to socket/bind/listen
```

**Solution:**

Ensure `ws2_32` library is linked in `CMakeLists.txt`:
```cmake
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()
```

## Python Environment Issues

### Module Not Found

**Symptom:**
```
ModuleNotFoundError: No module named 'gymnasium'
```

**Solution:**

1. **Install dependencies:**
   ```bash
   cd agent
   uv sync
   ```

2. **Verify installation:**
   ```bash
   uv run python -c "import gymnasium; print(gymnasium.__version__)"
   ```

### Import Errors

**Symptom:**
```
ImportError: cannot import name 'StarshipEnv' from 'agent.envs'
```

**Solution:**

1. **Check Python path:**
   The test script should handle this, but if running manually:
   ```python
   import sys
   from pathlib import Path
   sys.path.insert(0, str(Path(__file__).parent.parent.parent))
   ```

2. **Verify file structure:**
   ```bash
   ls -la agent/envs/starship_env.py
   ls -la agent/envs/__init__.py
   ```

## Training Issues

### No Reward Signal

**Symptom:**
```
Episode reward: 0.0
Mean reward: 0.0 ± 0.0
```

**Solution:**

1. **Check game state is being sent:**
   Add debug logging in `src/main.c`:
   ```c
   SDL_Log("Sending state: reward=%.2f, game_over=%d", reward, game_state->game_over);
   ```

2. **Verify reward calculation:**
   The reward function is in `src/game.c:450-499`

3. **Monitor with prints:**
   In `starship_env.py`:
   ```python
   def _receive_state(self):
       # ... existing code ...
       print(f"Received reward: {reward}, game_over: {terminated}")
       return obs, reward, terminated, truncated
   ```

### Agent Not Learning

**Symptom:**
```
Average episode reward not increasing after many episodes
```

**Solutions:**

1. **Check observation space:**
   ```python
   obs, _ = env.reset()
   print(f"Observation: {obs}")
   print(f"Min: {obs.min()}, Max: {obs.max()}")
   ```
   Values should be normalized (roughly in [0, 1] range)

2. **Increase training time:**
   ```bash
   uv run python scripts/train.py --timesteps 5000000
   ```

3. **Tune hyperparameters:**
   Edit `scripts/train.py`:
   ```python
   model = PPO(
       "MlpPolicy",
       env,
       learning_rate=1e-4,  # Try different values
       ent_coef=0.01,       # Higher = more exploration
       ...
   )
   ```

4. **Simplify reward:**
   In `src/game.c`, temporarily use only:
   ```c
   float reward = 0.0;
   if (!state->game_over) {
       reward += 1.0;  // Just survive
   } else {
       reward -= 100.0;  // Collision penalty
   }
   return reward;
   ```

### Game Crashes During Training

**Symptom:**
```
Lost connection to RL agent
Segmentation fault
```

**Solutions:**

1. **Check for null pointers:**
   In `src/main.c`, ensure `game_state` is valid before use

2. **Validate asteroid data:**
   ```c
   if (active_asteroids >= 10) {
       SDL_Log("Warning: Too many asteroids, limiting to 10");
       active_asteroids = 10;
   }
   ```

3. **Add error handling:**
   ```c
   if (!bridge_send_state(json_state)) {
       SDL_Log("Failed to send state, disconnecting");
       return SDL_APP_SUCCESS;
   }
   ```

## Performance Issues

### Training Very Slow

**Solutions:**

1. **Use headless mode:**
   ```bash
   ./build/starship_game --rl-mode --headless
   ```

2. **Check CPU usage:**
   ```bash
   # While training is running
   top -pid $(pgrep starship_game)
   ```

3. **Optimize observation space:**
   Reduce `max_asteroids` in `starship_env.py`:
   ```python
   self.max_asteroids = 5  # Instead of 10
   ```

4. **Use GPU:**
   ```bash
   # Check PyTorch GPU support
   uv run python -c "import torch; print(torch.cuda.is_available())"
   ```

### High Memory Usage

**Solutions:**

1. **Limit buffer size:**
   In `scripts/train.py`:
   ```python
   model = PPO(
       "MlpPolicy",
       env,
       n_steps=1024,  # Reduce from 2048
       ...
   )
   ```

2. **Reduce replay buffer:**
   ```python
   model = PPO(
       "MlpPolicy",
       env,
       buffer_size=50000,  # Add this
       ...
   )
   ```

## Testing & Debugging

### Enable Debug Logging

**Game (C):**
```bash
# Run with SDL debug output
SDL_LOG_PRIORITY=debug ./build/starship_game --rl-mode
```

**Python:**
```python
import logging
logging.basicConfig(level=logging.DEBUG)
```

### Manual Connection Test

**Terminal 1 - Start game:**
```bash
./build/starship_game --rl-mode
```

**Terminal 2 - Test with netcat:**
```bash
# Connect
nc localhost 5555

# Send reset action (4 bytes, value -1)
echo -ne '\xff\xff\xff\xff' | nc localhost 5555
```

### Check Port Status

**macOS/Linux:**
```bash
# List all processes using port 5555
lsof -i :5555

# Kill process on port 5555
kill -9 $(lsof -t -i:5555)
```

**Windows:**
```powershell
# Find process using port 5555
netstat -ano | findstr :5555

# Kill process (replace PID)
taskkill /PID <pid> /F
```

## Getting Help

If you're still stuck:

1. **Run the test script:**
   ```bash
   cd agent
   uv run python scripts/test_connection.py
   ```

2. **Check logs:**
   - Game logs: Look for SDL_Log output
   - Training logs: Check `agent/logs/` directory
   - Tensorboard: `uv run tensorboard --logdir logs`

3. **Verify versions:**
   ```bash
   uv run python -c "import gymnasium; import stable_baselines3; print(f'Gym: {gymnasium.__version__}, SB3: {stable_baselines3.__version__}')"
   ```

4. **Clean reinstall:**
   ```bash
   # Python environment
   cd agent
   rm -rf .venv uv.lock
   uv sync

   # Game build
   cd ..
   rm -rf build
   cmake -B build && cmake --build build
   ```