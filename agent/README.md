# Starship RL Agent

This directory contains reinforcement learning (RL) code for training an agent to play the Starship asteroid avoidance game using OpenAI Gymnasium (formerly OpenAI Gym) and Stable-Baselines3.

## Directory Structure

```
agent/
├── envs/              # Gymnasium environment wrapper
│   ├── starship_env.py    # Main environment implementation
│   └── __init__.py
├── scripts/           # Training and evaluation scripts
│   ├── train.py       # PPO training script
│   └── evaluate.py    # Model evaluation script
├── utils/             # C bridge for game communication
│   ├── game_bridge.c  # Socket-based game interface
│   └── game_bridge.h
├── models/            # Saved models (created during training)
└── logs/              # Tensorboard logs (created during training)
```

## Setup

### 1. Install Python Dependencies

This project uses [uv](https://docs.astral.sh/uv/) for fast, reliable Python package management.

```bash
# Install uv if you haven't already
curl -LsSf https://astral.sh/uv/install.sh | sh

# Sync dependencies (from agent/ directory)
cd agent
uv sync
```

### 2. Test the Connection

Verify the game bridge works correctly:

```bash
cd agent

# Basic connection test
uv run python scripts/test_connection.py

# Test automatic reset across episodes
uv run python scripts/test_reset.py
```

This will start the game, connect to it, and verify automatic resets work properly.

### 3. Build the Game with RL Support

The game needs to be modified to support RL mode by integrating the game bridge:

1. Include `agent/utils/game_bridge.h` in your game code
2. Add `--rl-mode` and `--headless` command-line argument support
3. Integrate bridge communication in the game loop:
   - Initialize bridge with `bridge_init(port)`
   - Accept connections with `bridge_accept_connection()`
   - Receive actions with `bridge_receive_action()`
   - Send game state with `bridge_send_state(json_state)`

### 4. Compile with Bridge Support

The game bridge is already integrated! Just build normally:

```bash
cmake -B build && cmake --build build
```

## Quick Start

The easiest way to start training:

```bash
# From the agent/ directory

# Headless training (faster, no window)
./scripts/quick_start.sh

# With rendering (see the game while training)
./scripts/quick_start.sh --render
```

This will:
1. Build the game if needed
2. Install dependencies with uv
3. Start the game in RL mode
4. Begin training automatically

**Note:** Headless mode is faster for training, but `--render` lets you watch the agent learn in real-time!

## Training

Train a new agent using PPO (Proximal Policy Optimization):

```bash
# Headless (faster)
uv run python scripts/train.py

# With rendering (watch the agent learn)
uv run python scripts/train.py --render
```

**Note:** The environment automatically starts the game - no need to run it separately!

### Training Options

```bash
uv run python scripts/train.py \
    --timesteps 1000000 \    # Total training steps (default: 1M)
    --lr 3e-4 \              # Learning rate (default: 3e-4)
    --render \               # Show game window during training (default: headless)
    --save-dir models \      # Model save directory
    --log-dir logs           # Tensorboard log directory
```

### Watching Training Progress

**See the game window:**
```bash
uv run python scripts/train.py --render
```

**Monitor metrics with Tensorboard:**
```bash
# In another terminal
uv run tensorboard --logdir logs
# Open http://localhost:6006 in your browser
```

## Evaluation

Evaluate a trained model:

```bash
uv run python scripts/evaluate.py models/starship_ppo_final.zip
```

### Evaluation Options

```bash
uv run python scripts/evaluate.py models/starship_ppo_final.zip \
    --episodes 10 \      # Number of test episodes (default: 10)
    --no-render \        # Disable rendering
    --stochastic \       # Use stochastic policy (default: deterministic)
    --interactive        # Continuous interactive play mode
```

### Interactive Play

Watch the agent play continuously:

```bash
uv run python scripts/evaluate.py models/starship_ppo_final.zip --interactive
```

## Environment Details

### Observation Space

The agent observes a 54-dimensional vector containing:
- **Starship state** (4 values): `[x, y, vx, vy]` (normalized)
- **Asteroid states** (5 values × 10 asteroids): `[x, y, vx, vy, radius]` (normalized)

All coordinates are normalized to [0, 1] range.

### Action Space

**Discrete actions** (5 options):
- 0: Move UP
- 1: Move DOWN
- 2: Move LEFT
- 3: Move RIGHT
- 4: NO-OP (no operation)

### Reward Function

The reward function should encourage:
- **Survival**: +1 per frame alive
- **Collision penalty**: -100 for hitting asteroids
- **Proximity awareness**: Small negative reward for being close to asteroids
- **Screen bounds**: Small penalty for approaching screen edges

(Implement reward logic in the C game code)

## Customization

### Hyperparameters

Modify hyperparameters in `scripts/train.py`:
- `learning_rate`: Step size for gradient updates
- `n_steps`: Rollout buffer size
- `batch_size`: Mini-batch size
- `gamma`: Discount factor for future rewards
- `ent_coef`: Entropy coefficient for exploration

### Network Architecture

The default policy uses a Multi-Layer Perceptron (MLP). To customize:

```python
from stable_baselines3 import PPO

model = PPO(
    "MlpPolicy",
    env,
    policy_kwargs=dict(
        net_arch=[256, 256, 128]  # Custom network layers
    ),
    ...
)
```

### Continuous Actions

To use continuous control instead of discrete actions:

1. In `envs/starship_env.py`, uncomment:
```python
self.action_space = spaces.Box(low=-1.0, high=1.0, shape=(2,), dtype=np.float32)
```

2. Modify the game bridge to accept continuous values

## Troubleshooting

### Connection Issues

If the Python environment can't connect to the game:

**Test the connection first:**
```bash
cd agent
uv run python scripts/test_connection.py
```

**Common issues:**
- **Game not built**: Run `cmake -B build && cmake --build build` from project root
- **Port in use**: Check with `lsof -i :5555` (macOS/Linux) or `netstat -an | grep 5555` (Windows)
- **Firewall blocking**: Allow localhost connections on port 5555
- **Path issues**: The environment auto-detects the game path, but you can specify it manually:
  ```python
  env = StarshipEnv(render_mode=None)
  # Game path is auto-detected from agent/envs directory
  ```

### Training Not Converging

Try adjusting:
- Increase `total_timesteps` (1M → 5M+)
- Tune reward function for better feedback
- Adjust `learning_rate` (try 1e-4 or 1e-3)
- Increase `ent_coef` for more exploration

### Performance Issues

- Train in headless mode (without `--render`)
- Use GPU acceleration if available (PyTorch with CUDA)
- Reduce observation space complexity if needed

## Advanced Features

### Parallel Training

Use vectorized environments for faster training:

```python
from stable_baselines3.common.env_util import make_vec_env

env = make_vec_env(lambda: StarshipEnv(), n_envs=4)
```

### Custom Callbacks

Add custom callbacks for logging or early stopping:

```python
from stable_baselines3.common.callbacks import BaseCallback

class CustomCallback(BaseCallback):
    def _on_step(self):
        # Custom logic here
        return True
```

## References

- [Stable-Baselines3 Documentation](https://stable-baselines3.readthedocs.io/)
- [Gymnasium Documentation](https://gymnasium.farama.org/)
- [PPO Paper](https://arxiv.org/abs/1707.06347)