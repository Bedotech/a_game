# Starship RL Training & Evaluation Guide

Complete guide for training and evaluating reinforcement learning agents for the Starship game.

---

## 🚀 Quick Start

### 1. Train an Agent (Fast - Recommended)
```bash
# Train with 4 parallel environments at 2x speed
python -m agent.scripts.train --n-envs 4 --speed 2.0 --timesteps 1000000
```

### 2. Watch Your Trained Agent Play
```bash
# Interactive mode - watch the agent play continuously
python -m agent.scripts.evaluate models/starship_ppo_final --interactive
```

---

## 📚 Training Options

### Basic Training
```bash
# Minimal training (1M steps, single environment)
python -m agent.scripts.train --timesteps 1000000
```

### Fast Training (Recommended)
```bash
# 4 parallel environments at 2x speed
python -m agent.scripts.train \
    --n-envs 4 \
    --speed 2.0 \
    --timesteps 1000000

# Maximum speed: 8 environments at 3x speed
python -m agent.scripts.train \
    --n-envs 8 \
    --speed 3.0 \
    --timesteps 1000000
```

### Training with Visualization
```bash
# Watch the first environment while training
python -m agent.scripts.train \
    --n-envs 4 \
    --render \
    --timesteps 100000
```

### Custom Training Parameters
```bash
python -m agent.scripts.train \
    --timesteps 2000000 \       # Total training steps
    --n-envs 4 \                # Parallel environments
    --speed 2.0 \               # Game speed multiplier
    --lr 0.0003 \               # Learning rate
    --save-dir models \         # Model save directory
    --log-dir logs              # TensorBoard logs
```

### Training Command-Line Arguments

| Argument | Default | Description |
|----------|---------|-------------|
| `--timesteps` | 1000000 | Total training timesteps |
| `--n-envs` | 1 | Number of parallel environments (recommended: 4-8) |
| `--speed` | 2.0 | Game speed multiplier (1.0 = normal, 2.0 = 2x faster) |
| `--lr` | 0.0003 | Learning rate |
| `--render` | False | Show game window during training |
| `--save-dir` | models | Directory to save trained models |
| `--log-dir` | logs | Directory for TensorBoard logs |
| `--enable-eval` | False | Enable evaluation callback (optional) |

---

## 📊 Monitoring Training

### TensorBoard (Real-time Metrics)
```bash
# Start TensorBoard
tensorboard --logdir logs

# Open browser to: http://localhost:6006
```

**Available Metrics:**
- `rollout/ep_rew_mean` - Average episode reward
- `rollout/ep_len_mean` - Average episode length
- `train/loss` - Training loss
- `train/entropy_loss` - Exploration entropy

### Checkpoints

Models are automatically saved during training:
- **Checkpoints**: `models/starship_ppo_10000_steps.zip` (every 10k steps)
- **Final model**: `models/starship_ppo_final.zip` (when training completes)

---

## 🎮 Evaluating Your Agent

### Interactive Mode (Watch Agent Play)
```bash
# Watch agent play continuously (Ctrl+C to stop)
python -m agent.scripts.evaluate models/starship_ppo_final --interactive

# Watch at 2x speed
python -m agent.scripts.evaluate models/starship_ppo_final --interactive --speed 2.0
```

### Performance Evaluation (Statistics)
```bash
# Evaluate agent performance over 10 episodes
python -m agent.scripts.evaluate models/starship_ppo_final --episodes 10

# Evaluate over 50 episodes for more accurate statistics
python -m agent.scripts.evaluate models/starship_ppo_final --episodes 50
```

### Testing Specific Checkpoints
```bash
# Test an early checkpoint
python -m agent.scripts.evaluate models/starship_ppo_10000_steps --interactive

# Compare final vs checkpoint
python -m agent.scripts.evaluate models/starship_ppo_100000_steps --episodes 20
python -m agent.scripts.evaluate models/starship_ppo_final --episodes 20
```

### Evaluation Command-Line Arguments

| Argument | Default | Description |
|----------|---------|-------------|
| `model_path` | *required* | Path to trained model file |
| `--episodes` | 10 | Number of episodes to evaluate |
| `--interactive` | False | Watch agent play continuously |
| `--speed` | 1.0 | Game speed multiplier |
| `--no-render` | False | Disable visualization (headless mode) |
| `--stochastic` | False | Use stochastic actions (adds randomness) |

---

## 🔧 Performance Optimization

### Parallel Environments Scaling

| Environments | Expected Speedup | CPU Usage | Recommended For |
|--------------|------------------|-----------|-----------------|
| 1 | 1x (baseline) | Low | Debugging, watching agent |
| 4 | ~4x | Medium | Standard training |
| 8 | ~8x | High | Fast training |
| 16 | ~12-16x | Very High | Maximum speed (powerful CPUs) |

### Speed Multiplier Effect

| Speed | Training Time | Episode Difficulty | Use Case |
|-------|---------------|-------------------|----------|
| 1.0x | Normal | Normal | Debugging, visualization |
| 2.0x | 50% faster | Same | Recommended for training |
| 3.0x | 66% faster | Same | Fast training |
| 5.0x | 80% faster | Same | Very fast (may reduce stability) |

### Combined Optimization Example
```bash
# Maximum training speed: 8 envs × 2x speed = ~16x faster
python -m agent.scripts.train --n-envs 8 --speed 2.0 --timesteps 2000000

# Expected: ~2M steps in ~30-60 minutes (vs 8-16 hours without optimization)
```

---

## 📁 Directory Structure

```
agent/
├── envs/
│   └── starship_env.py          # Gym environment wrapper
├── scripts/
│   ├── train.py                 # Training script
│   └── evaluate.py              # Evaluation script
├── models/                       # Saved models (auto-created)
│   ├── starship_ppo_10000_steps.zip
│   ├── starship_ppo_20000_steps.zip
│   └── starship_ppo_final.zip
└── logs/                         # TensorBoard logs (auto-created)
    └── PPO_1/
```

---

## 🎯 Training Tips

### For Best Results:

1. **Start with moderate parallelism**: 4-8 environments
2. **Use 2x speed multiplier**: Faster training without instability
3. **Train for 1M+ steps**: Usually needed for good performance
4. **Monitor TensorBoard**: Watch for reward improvements
5. **Test checkpoints**: Evaluate intermediate models to track progress

### Common Issues:

**Slow training (< 100 it/s):**
- Increase `--n-envs` to 4-8
- Increase `--speed` to 2.0
- Check CPU usage (may need fewer envs if maxed out)

**Agent not improving:**
- Train longer (2M+ steps)
- Adjust learning rate (`--lr 0.0001` or `--lr 0.001`)
- Check TensorBoard for entropy (should decrease slowly)

**Port conflicts:**
- Each environment uses a unique port (5555, 5556, etc.)
- Make sure ports are available
- Kill old game processes: `pkill starship_game`

---

## 🎬 Example Workflow

```bash
# 1. Quick training run (small test)
python -m agent.scripts.train --n-envs 4 --timesteps 100000

# 2. Check the agent's performance
python -m agent.scripts.evaluate models/starship_ppo_final --interactive

# 3. Full training (if satisfied with quick test)
python -m agent.scripts.train --n-envs 8 --speed 2.0 --timesteps 2000000

# 4. Monitor progress
tensorboard --logdir logs

# 5. Final evaluation
python -m agent.scripts.evaluate models/starship_ppo_final --episodes 50

# 6. Watch it play!
python -m agent.scripts.evaluate models/starship_ppo_final --interactive --speed 1.5
```

---

## 📈 Expected Training Timeline

| Configuration | Training Time* | Final Performance** |
|---------------|---------------|---------------------|
| 1 env, 1x speed | ~8 hours | Good |
| 4 envs, 2x speed | ~1 hour | Good |
| 8 envs, 2x speed | ~30 min | Good |
| 8 envs, 3x speed | ~20 min | Good |

\* For 1M timesteps
\*\* "Good" = agent survives 100+ steps consistently

---

## 🤖 Understanding Model Files

**Checkpoint naming:**
- `starship_ppo_10000_steps.zip` = Saved at 10,000 training steps
- `starship_ppo_final.zip` = Final model after all training

**Model contains:**
- Neural network weights
- Policy and value function parameters
- Normalization statistics

**File size:** ~1-5 MB per model

---

Happy training! 🚀
