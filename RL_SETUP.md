# Reinforcement Learning Setup

This project now supports training RL agents using OpenAI Gymnasium and Stable-Baselines3!

## What's New

✅ **Game Bridge**: C socket-based communication system for RL training
✅ **Gymnasium Environment**: Python wrapper for the Starship game
✅ **PPO Training**: Ready-to-use training scripts with Stable-Baselines3
✅ **Reward System**: Automatic reward calculation based on survival and performance
✅ **Headless Mode**: Train without rendering for faster learning

## Quick Start

```bash
# 1. Build the game with RL support (already done!)
cmake -B build && cmake --build build

# 2. Install Python dependencies
cd agent
uv sync

# 3. Start training!
# Headless (faster)
./scripts/quick_start.sh

# OR with rendering (watch the agent learn)
./scripts/quick_start.sh --render
```

That's it! The training will begin automatically.

## Project Structure

```
a_game/
├── agent/                      # RL training code
│   ├── envs/                   # Gymnasium environment
│   │   └── starship_env.py    # Main environment wrapper
│   ├── scripts/                # Training & evaluation
│   │   ├── train.py           # PPO training script
│   │   ├── evaluate.py        # Model evaluation
│   │   └── quick_start.sh     # One-command startup
│   ├── utils/                  # C bridge for game communication
│   │   ├── game_bridge.c      # Socket server implementation
│   │   └── game_bridge.h      # Header file
│   ├── models/                 # Saved models (created during training)
│   ├── logs/                   # Tensorboard logs (created during training)
│   ├── pyproject.toml          # Python dependencies (uv)
│   ├── README.md              # Detailed agent documentation
│   └── INTEGRATION.md         # Technical integration details
│
├── src/                        # Game source code
│   ├── main.c                 # Modified for RL mode support
│   └── game.c                 # Added reward calculation
│
└── build/
    └── starship_game          # Executable with RL support

```

## How It Works

1. **Game runs in RL mode**: `./build/starship_game --rl-mode --headless`
2. **Socket communication**: Game listens on port 5555
3. **Python agent connects**: Sends actions, receives state + reward
4. **PPO learns**: Optimizes policy to maximize cumulative reward
5. **Model saves**: Best models saved to `agent/models/`

## Command Reference

### Running the Game

```bash
# Normal game (human play)
./build/starship_game

# RL mode with rendering
./build/starship_game --rl-mode

# RL mode headless (training)
./build/starship_game --rl-mode --headless

# Custom port
./build/starship_game --rl-mode --port=5556
```

### Training

```bash
cd agent

# Quick start (headless - faster)
./scripts/quick_start.sh

# Quick start with rendering (watch it learn!)
./scripts/quick_start.sh --render

# Manual training (headless)
uv run python scripts/train.py --timesteps 1000000

# Manual training with rendering
uv run python scripts/train.py --render
```

### Evaluation

```bash
cd agent

# Evaluate a trained model
uv run python scripts/evaluate.py models/starship_ppo_final.zip

# Interactive play mode
uv run python scripts/evaluate.py models/best_model.zip --interactive
```

### Monitoring

```bash
cd agent

# Start Tensorboard
uv run tensorboard --logdir logs

# Open in browser: http://localhost:6006
```

## Observation Space

The agent observes a **54-dimensional vector**:
- **Starship state** (4 values): `[x, y, vx, vy]`
- **Up to 10 asteroids** (5 values each): `[x, y, vx, vy, radius]`

All values are normalized to `[0, 1]` range.

## Action Space

**5 discrete actions:**
- 0: Move UP
- 1: Move DOWN
- 2: Move LEFT
- 3: Move RIGHT
- 4: NO-OP (do nothing)

## Reward Function

| Event | Reward |
|-------|--------|
| Staying alive | +1.0 per frame |
| Game over (collision) | -100.0 |
| Score increase | +10.0 × points |
| Near screen edge | -0.1 |
| Close to asteroid | -0.5 × proximity |

Rewards are calculated in `src/game.c:450-499`.

## Training Tips

### For Faster Training

1. **Use headless mode**: `--headless` flag removes rendering overhead
2. **Increase timesteps**: Try `--timesteps 5000000` or more
3. **Parallel environments**: Edit `train.py` to use vectorized envs
4. **GPU acceleration**: Ensure PyTorch is using CUDA if available

### If Agent Isn't Learning

1. **Check reward scale**: Monitor Tensorboard graphs
2. **Tune hyperparameters**: Adjust learning rate, entropy coefficient
3. **Increase exploration**: Higher `ent_coef` in training script
4. **Simplify environment**: Start with fewer asteroids
5. **Longer training**: Some games need 10M+ timesteps

### Customize Rewards

Edit `game_state_calculate_reward()` in `src/game.c` to:
- Change reward magnitudes
- Add new reward signals
- Implement curriculum learning
- Shape rewards for specific behaviors

Then rebuild: `cmake --build build`

## Documentation

- **`agent/README.md`**: Detailed RL agent documentation
- **`agent/INTEGRATION.md`**: Technical integration details
- **`CLAUDE.md`**: Original project overview

## Examples

### Train for 5 million steps

```bash
cd agent
uv run python scripts/train.py --timesteps 5000000 --log-dir logs/long_training
```

### Evaluate with 20 episodes

```bash
cd agent
uv run python scripts/evaluate.py models/best_model.zip --episodes 20
```

### Monitor training progress

```bash
cd agent
uv run tensorboard --logdir logs
# Open browser to http://localhost:6006
```

## Troubleshooting

### "Connection refused" error

**Problem**: Python can't connect to game
**Solution**: Start game with `--rl-mode` flag first

### Game hangs on startup

**Problem**: Game waits for agent connection
**Solution**: Start Python training script to connect

### Build errors with game_bridge.c

**Problem**: CMake can't find game bridge
**Solution**: Ensure `agent/utils/game_bridge.c` exists, rebuild from scratch:
```bash
rm -rf build && cmake -B build && cmake --build build
```

### Training is very slow

**Problem**: Training taking too long
**Solution**:
- Use `--headless` mode
- Reduce network complexity in `train.py`
- Check CPU/GPU utilization

## Advanced Usage

### Custom Network Architecture

Edit `agent/scripts/train.py`:

```python
model = PPO(
    "MlpPolicy",
    env,
    policy_kwargs=dict(
        net_arch=[256, 256, 128]  # Customize layers
    ),
    ...
)
```

### Different Algorithms

Try SAC, DQN, or A2C instead of PPO:

```python
from stable_baselines3 import SAC

model = SAC("MlpPolicy", env, ...)
```

### Parallel Training

Use multiple game instances:

```python
from stable_baselines3.common.env_util import make_vec_env

env = make_vec_env(lambda: StarshipEnv(port=5555+i), n_envs=4)
```

## Contributing

To add new features:

1. **Add observations**: Modify `bridge_build_state_json()` in `game_bridge.c`
2. **Add actions**: Update `game_state_apply_rl_action()` in `game.c`
3. **Tune rewards**: Edit `game_state_calculate_reward()` in `game.c`
4. **Update environment**: Modify `StarshipEnv` in `starship_env.py`

## License

Same as the main project.

---

Happy training! 🚀🤖