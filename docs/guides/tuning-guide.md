# 参数调优指南

## 调参方式

按 **HOME** 打开覆盖层，所有参数实时生效，无需重载。关掉 `ai.exe` 自动保存到 `config.ini`。

> 备份配置: `copy build\dml\Release\config.ini my_backup.ini`

---

## 核心参数速查

### 检测 (AI 模型标签页)

| 参数 | 默认 | 说明 |
|------|------|------|
| `confidence_threshold` | 0.20 | 越低框越多（含误检），越高框越少（只留准的） |
| `nms_threshold` | 0.40 | 重叠框去重，越高保留越多重叠框 |
| `detection_resolution` | 320 | 160=最快/最不准, 640=最慢/最准 |

**调法**：靶场里拖 `confidence_threshold`，找到"刚好不漏人、也没有杂框"的点。先定这个，再动其他。

### 鼠标速度 (移动标签页)

| 参数 | 说明 |
|------|------|
| `minSpeedMultiplier` | 近距微调速度，太高会抖 |
| `maxSpeedMultiplier` | 远距大角度拉枪速度 |
| `snapRadius` | 小于此距离直接瞬移贴上去 |
| `nearRadius` | 此范围内开始减速 |
| `speedCurveExponent` | 速度曲线形状，越大越"先慢后快" |
| `snapBoostFactor` | 进入 snap 区的额外加速 |

**RP2040 硬件鼠标** 像素直出，值要远小于 WIN32：

| 输入方式 | 推荐速度范围 |
|----------|------------|
| RP2350 (硬件) | 0.02 ~ 0.25 |
| WIN32 | 0.10 ~ 1.00 |

### 预测 (预测标签页)

| 参数 | 静止靶 | 移动靶 |
|------|--------|--------|
| `kalman_enabled` | **关** | 开 |
| `predictionInterval` | 0 | 0.01 |
| `prediction_futurePositions` | 1 | 3~5 |

**静止靶子务必关卡尔曼**，否则鼠标会"瞄到又跑掉"。

---

## 分场景调参流程

### 场景一：静止靶（打靶场）

```
1. 关卡尔曼: kalman_enabled = false
2. 关预测: predictionInterval = 0
3. 调 speed 到跟手不抖
4. 调 confidence 到只标人不标杂物
5. 满意 → 关 ai.exe 自动保存
```

### 场景二：移动靶

```
1. 从静止靶配置出发
2. 开卡尔曼: kalman_enabled = true
3. 开预测: predictionInterval = 0.01, futurePositions = 3
4. 如果"瞄到又跑掉"→ 降 kalman_process_noise_velocity
5. 如果"跟不上"→ 升 maxSpeedMultiplier
```

### 场景三：远距离敌人

远距离目标在屏幕上很小，模型检测更慢更不准：

```
1. detection_resolution = 640（牺牲速度换精度）
2. confidence_threshold = 0.15（降阈值多检出）
3. maxSpeedMultiplier 提升 30-50%（远距离需要更大位移）
4. 确认 fovX/fovY 匹配游戏实际 FOV
```

---

## 常见问题速查

| 问题 | 可能原因 | 解决方法 |
|------|---------|---------|
| 杂框太多 | 置信度过低 | 升 `confidence_threshold` |
| 不漏人 | 置信度过高 | 降 `confidence_threshold` |
| 瞄到又移走 | 卡尔曼过拟合 | 关 `kalman_enabled` |
| 鼠标抖动 | 速度过快 | 降 `minSpeedMultiplier` |
| 跟不手 | 速度过慢 | 升 `maxSpeedMultiplier` |
| 90° 大角度飘 | snap 太激进 | 降 `snapRadius` |
| 只瞄身体不瞄头 | head 类未检出 | 手动标 head 数据重训 |
| 远距离不检测 | 分辨率太低 | 升 `detection_resolution` |

---

## 一次只改一个

调参最忌讳同时动好几个。正确姿势：

```
改一个参数 → 打 30 秒感受 → 不好 → 改回去 → 换下一个
                                    → 好 → 记录 → 换下一个
```

调好的参数及时备份：

```powershell
copy build\dml\Release\config.ini config_deltaforce.ini
```
