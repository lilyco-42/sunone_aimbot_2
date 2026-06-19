#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include <winsock2.h>
#include <Windows.h>

#include <shellapi.h>
#include <algorithm>

#include "imgui/imgui.h"
#include <imgui_internal.h>

#include "sunone_aimbot_2.h"
#include "overlay/config_dirty.h"
#include "overlay/ui_sections.h"
#include "include/other_tools.h"
#include "kmbox_net/picture.h"

std::string ghub_version = get_ghub_version();

int prev_fovX = config.fovX;
int prev_fovY = config.fovY;
float prev_minSpeedMultiplier = config.minSpeedMultiplier;
float prev_maxSpeedMultiplier = config.maxSpeedMultiplier;
float prev_predictionInterval = config.predictionInterval;
bool  prev_kalman_enabled = config.kalman_enabled;
float prev_kalman_process_noise_position = config.kalman_process_noise_position;
float prev_kalman_process_noise_velocity = config.kalman_process_noise_velocity;
float prev_kalman_measurement_noise = config.kalman_measurement_noise;
float prev_kalman_velocity_damping = config.kalman_velocity_damping;
float prev_kalman_max_velocity = config.kalman_max_velocity;
int   prev_kalman_warmup_frames = config.kalman_warmup_frames;
bool  prev_kalman_compensate_detection_delay = config.kalman_compensate_detection_delay;
float prev_kalman_additional_prediction_ms = config.kalman_additional_prediction_ms;
float prev_kalman_reset_timeout_sec = config.kalman_reset_timeout_sec;
float prev_snapRadius = config.snapRadius;
float prev_nearRadius = config.nearRadius;
float prev_speedCurveExponent = config.speedCurveExponent;
float prev_snapBoostFactor = config.snapBoostFactor;

bool  prev_wind_mouse_enabled = config.wind_mouse_enabled;
float prev_wind_G = config.wind_G;
float prev_wind_W = config.wind_W;
float prev_wind_M = config.wind_M;
float prev_wind_D = config.wind_D;

bool prev_auto_shoot = config.auto_shoot;
float prev_bScope_multiplier = config.bScope_multiplier;

namespace
{
enum class MouseSettingsPage
{
    All,
    Movement,
    Prediction,
    Assist,
    Profiles,
    Input
};

bool shouldDrawMousePage(MouseSettingsPage current, MouseSettingsPage wanted)
{
    return current == MouseSettingsPage::All || current == wanted;
}
}

static void draw_mouse_page(MouseSettingsPage page)
{
    if (shouldDrawMousePage(page, MouseSettingsPage::Movement) &&
        OverlayUI::BeginSection("视野", "mouse_section_fov"))
    {
        OverlayUI::SliderIntRow("视野 X", &config.fovX, 10, 120);
        OverlayUI::SliderIntRow("视野 Y", &config.fovY, 10, 120);
        OverlayUI::EndSection();
    }

    if (shouldDrawMousePage(page, MouseSettingsPage::Movement) &&
        OverlayUI::BeginSection("速度系数", "mouse_section_speed_multiplier"))
    {
        OverlayUI::SliderFloatRow("最小速度系数", &config.minSpeedMultiplier, 0.1f, 5.0f, "%.1f");
        OverlayUI::SliderFloatRow("最大速度系数", &config.maxSpeedMultiplier, 0.1f, 5.0f, "%.1f");
        OverlayUI::EndSection();
    }

    if (shouldDrawMousePage(page, MouseSettingsPage::Prediction) &&
        OverlayUI::BeginSection("预测", "mouse_section_prediction"))
    {
        OverlayUI::SliderFloatRow("预测间隔", &config.predictionInterval, 0.00f, 0.5f, "%.2f");
        if (config.predictionInterval == 0.00f)
        {
            OverlayUI::TextRow("预测已禁用", IM_COL32(255, 108, 108, 255));
        }

        const bool predictionEnabled = (config.predictionInterval > 0.0f);
        if (!predictionEnabled)
        {
            ImGui::BeginDisabled();
        }
        
        if (OverlayUI::SliderIntRow("未来位置", &config.prediction_futurePositions, 1, 40))
        {
            OverlayConfig_MarkDirty();
        }
        
        if (OverlayUI::CheckboxRow("绘制未来位置", &config.draw_futurePositions))
        {
            OverlayConfig_MarkDirty();
        }
        
        if (!predictionEnabled)
        {
            ImGui::EndDisabled();
            ImGui::TextDisabled("启用预测间隔 (> 0) 以编辑此部分");
        }

        ImGui::Separator();
        if (OverlayUI::CheckboxRow("启用卡尔曼滤波", &config.kalman_enabled))
        {
            OverlayConfig_MarkDirty();
        }

        if (OverlayUI::SliderFloatRow("卡尔曼过程噪声位置", &config.kalman_process_noise_position, 0.001f, 5000.0f, "%.3f"))
        {
            OverlayConfig_MarkDirty();
        }

        if (OverlayUI::SliderFloatRow("卡尔曼过程噪声速度", &config.kalman_process_noise_velocity, 0.001f, 50000.0f, "%.3f"))
        {
            OverlayConfig_MarkDirty();
        }

        if (OverlayUI::SliderFloatRow("卡尔曼测量噪声", &config.kalman_measurement_noise, 0.001f, 5000.0f, "%.3f"))
        {
            OverlayConfig_MarkDirty();
        }

        if (OverlayUI::SliderFloatRow("卡尔曼速度阻尼", &config.kalman_velocity_damping, 0.0f, 3.0f, "%.3f"))
        {
            OverlayConfig_MarkDirty();
        }

        if (OverlayUI::SliderFloatRow("卡尔曼最大速度", &config.kalman_max_velocity, 100.0f, 60000.0f, "%.0f"))
        {
            OverlayConfig_MarkDirty();
        }

        if (OverlayUI::SliderIntRow("卡尔曼预热帧数", &config.kalman_warmup_frames, 0, 20))
        {
            OverlayConfig_MarkDirty();
        }

        if (OverlayUI::CheckboxRow("卡尔曼补偿推理延迟", &config.kalman_compensate_detection_delay))
        {
            OverlayConfig_MarkDirty();
        }

        if (OverlayUI::SliderFloatRow("卡尔曼额外预测 (ms)", &config.kalman_additional_prediction_ms, -80.0f, 120.0f, "%.1f"))
        {
            OverlayConfig_MarkDirty();
        }

        if (OverlayUI::SliderFloatRow("卡尔曼重置超时 (s)", &config.kalman_reset_timeout_sec, 0.05f, 3.0f, "%.2f"))
        {
            OverlayConfig_MarkDirty();
        }

        OverlayUI::EndSection();
    }

    if (shouldDrawMousePage(page, MouseSettingsPage::Movement) &&
        OverlayUI::BeginSection("目标修正", "mouse_section_target_correction"))
    {
        OverlayUI::SliderFloatRow("吸附半径", &config.snapRadius, 0.1f, 5.0f, "%.1f");
        OverlayUI::SliderFloatRow("近距半径", &config.nearRadius, 1.0f, 40.0f, "%.1f");
        OverlayUI::SliderFloatRow("速度曲线指数", &config.speedCurveExponent, 0.1f, 10.0f, "%.1f");
        OverlayUI::SliderFloatRow("吸附增强系数", &config.snapBoostFactor, 0.01f, 4.00f, "%.2f");
        OverlayUI::EndSection();
    }

    if (shouldDrawMousePage(page, MouseSettingsPage::Profiles) &&
        OverlayUI::BeginSection("游戏配置", "mouse_section_game_profile"))
    {
        std::vector<std::string> profile_names;
        for (const auto& kv : config.game_profiles)
            profile_names.push_back(kv.first);
        std::sort(profile_names.begin(), profile_names.end());

        static int selected_index = 0;
        for (size_t i = 0; i < profile_names.size(); ++i)
        {
            if (profile_names[i] == config.active_game)
            {
                selected_index = static_cast<int>(i);
                break;
            }
        }

        std::vector<const char*> profile_items;
        for (const auto& name : profile_names)
            profile_items.push_back(name.c_str());

        if (OverlayUI::ComboRow("当前游戏配置", &selected_index, profile_items.data(), static_cast<int>(profile_items.size())))
        {
            config.active_game = profile_names[selected_index];
            OverlayConfig_MarkDirty();
            globalMouseThread->updateConfig(
                config.detection_resolution,
                config.fovX,
                config.fovY,
                config.minSpeedMultiplier,
                config.maxSpeedMultiplier,
                config.predictionInterval,
                config.auto_shoot,
                config.bScope_multiplier
            );
        }

        const auto& gp = config.currentProfile();

        ImGui::Text("Current profile: %s", gp.name.c_str());
        ImGui::Text("Sens: %.4f", gp.sens);
        ImGui::Text("Yaw:  %.4f", gp.yaw);
        ImGui::Text("Pitch: %.4f", gp.pitch);
        ImGui::Text("FOV Scaled: %s", gp.fovScaled ? "true" : "false");

        if (gp.name != "UNIFIED")
        {
            Config::GameProfile& modifiable = config.game_profiles[gp.name];
            bool changed = false;

            float sens_f = static_cast<float>(modifiable.sens);
            float yaw_f = static_cast<float>(modifiable.yaw);
            float pitch_f = static_cast<float>(modifiable.pitch);
            float baseFOV_f = static_cast<float>(modifiable.baseFOV);

            changed |= OverlayUI::SliderFloatRow("灵敏度", &sens_f, 0.001f, 10.0f, "%.4f");
            changed |= OverlayUI::SliderFloatRow("偏航", &yaw_f, 0.001f, 0.1f, "%.4f");
            changed |= OverlayUI::SliderFloatRow("俯仰", &pitch_f, 0.001f, 0.1f, "%.4f");

            changed |= OverlayUI::CheckboxRow("视野缩放", &modifiable.fovScaled);
            if (modifiable.fovScaled)
            {
                changed |= OverlayUI::SliderFloatRow("基础视野", &baseFOV_f, 10.0f, 180.0f, "%.1f");
            }

            if (changed)
            {
                modifiable.sens = static_cast<double>(sens_f);
                modifiable.yaw = static_cast<double>(yaw_f);

                modifiable.pitch = static_cast<double>(pitch_f);

                modifiable.baseFOV = static_cast<double>(baseFOV_f);

                OverlayConfig_MarkDirty();
            }
        }

        OverlayUI::EndSection();
    }

    if (shouldDrawMousePage(page, MouseSettingsPage::Profiles) &&
        OverlayUI::BeginSection("管理配置", "mouse_section_manage_profiles"))
    {
        static char new_profile_name[64] = "";
        bool addProfile = false;
        {
            const auto row = OverlayUI::BeginSettingRow("新配置名称");
            const float buttonW = 96.0f;
            const float inputW = std::max(1.0f, row.controlWidth - buttonW - ImGui::GetStyle().ItemSpacing.x);
            ImGui::SetNextItemWidth(inputW);
            ImGui::InputText("##value", new_profile_name, sizeof(new_profile_name));
            ImGui::SameLine();
            addProfile = ImGui::Button("添加", ImVec2(buttonW, 0.0f));
            OverlayUI::EndSettingRow(row);
        }
        if (addProfile)
        {
            std::string name = std::string(new_profile_name);
            if (!name.empty() && config.game_profiles.count(name) == 0)
            {
                Config::GameProfile gp;
                gp.name = name;
                gp.sens = 1.0;
                gp.yaw = 0.022;
                gp.pitch = 0.022;
                gp.fovScaled = false;
                gp.baseFOV = 90.0;
                config.game_profiles[name] = gp;
                config.active_game = name;
                OverlayConfig_MarkDirty();
                new_profile_name[0] = '\0'; // clear
            }
        }

        const auto& gp = config.currentProfile();
        if (gp.name != "UNIFIED")
        {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(200, 50, 50, 255));
            if (OverlayUI::ButtonRow("Profile", "删除当前配置", "delete_current_profile"))
            {
                config.game_profiles.erase(gp.name);
                if (config.game_profiles.count("UNIFIED") != 0)
                    config.active_game = "UNIFIED";
                else if (!config.game_profiles.empty())
                    config.active_game = config.game_profiles.begin()->first;
                else
                    config.active_game = "UNIFIED";

                OverlayConfig_MarkDirty();
            }
            ImGui::PopStyleColor();
        }

        OverlayUI::EndSection();
    }

    if (shouldDrawMousePage(page, MouseSettingsPage::Assist) &&
        OverlayUI::BeginSection("简易无后坐力", "mouse_section_easy_no_recoil"))
    {
        if (OverlayUI::CheckboxRow("简易无后坐力", &config.easynorecoil))
        {
            OverlayConfig_MarkDirty();
        }

        if (!config.easynorecoil)
        {
            ImGui::BeginDisabled();
        }

        if (OverlayUI::SliderFloatRow("无后坐力强度", &config.easynorecoilstrength, 0.1f, 500.0f, "%.1f"))
        {
            OverlayConfig_MarkDirty();
        }

        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "左右方向键：调整后坐力强度 ±10");

        if (config.easynorecoilstrength >= 100.0f)
        {
            ImGui::TextColored(ImVec4(255, 255, 0, 255), "警告：高后坐力强度可能被检测");
        }

        if (!config.easynorecoil)
        {
            ImGui::EndDisabled();
            ImGui::TextDisabled("启用简易无后坐力以编辑设置");
        }

        OverlayUI::EndSection();
    }

    if (shouldDrawMousePage(page, MouseSettingsPage::Assist) &&
        OverlayUI::BeginSection("自动射击", "mouse_section_auto_shoot"))
    {
        OverlayUI::CheckboxRow("自动射击", &config.auto_shoot);
        if (!config.auto_shoot)
        {
            ImGui::BeginDisabled();
        }

        OverlayUI::SliderFloatRow("瞄准镜系数", &config.bScope_multiplier, 0.5f, 2.0f, "%.1f");

        if (!config.auto_shoot)
        {
            ImGui::EndDisabled();
            ImGui::TextDisabled("启用自动射击以编辑设置");
        }

        OverlayUI::EndSection();
    }

    if (shouldDrawMousePage(page, MouseSettingsPage::Movement) &&
        OverlayUI::BeginSection("风动鼠标", "mouse_section_wind_mouse"))
    {
        if (OverlayUI::CheckboxRow("启用风动鼠标", &config.wind_mouse_enabled))
        {
            OverlayConfig_MarkDirty();
        }

        if (!config.wind_mouse_enabled)
        {
            ImGui::BeginDisabled();
        }

        if (OverlayUI::SliderFloatRow("重力", &config.wind_G, 4.00f, 40.00f, "%.2f"))
        {
            OverlayConfig_MarkDirty();
        }

        if (OverlayUI::SliderFloatRow("风力波动", &config.wind_W, 1.00f, 40.00f, "%.2f"))
        {
            OverlayConfig_MarkDirty();
        }

        if (OverlayUI::SliderFloatRow("最大步长（速度限制）", &config.wind_M, 1.00f, 40.00f, "%.2f"))
        {
            OverlayConfig_MarkDirty();
        }

        if (OverlayUI::SliderFloatRow("行为变化距离", &config.wind_D, 1.00f, 40.00f, "%.2f"))
        {
            OverlayConfig_MarkDirty();
        }

        if (OverlayUI::ButtonRow("风动鼠标", "重置默认值", "reset_wind_mouse_defaults"))
        {
            config.wind_G = 18.0f;
            config.wind_W = 15.0f;
            config.wind_M = 10.0f;
            config.wind_D = 8.0f;
            OverlayConfig_MarkDirty();
        }

        if (!config.wind_mouse_enabled)
        {
            ImGui::EndDisabled();
            ImGui::TextDisabled("启用风动鼠标以编辑设置");
        }

        OverlayUI::EndSection();
    }

    if (shouldDrawMousePage(page, MouseSettingsPage::Input) &&
        OverlayUI::BeginSection("输入方式", "mouse_section_input_method"))
    {
        std::vector<std::string> input_methods = { "WIN32", "GHUB", "RAZER", "ARDUINO", "RP2350", "TEENSY41", "TEENSY41_HID", "KMBOX_NET", "KMBOX_A", "MAKCU" };

        std::vector<const char*> method_items;
        method_items.reserve(input_methods.size());
        for (const auto& item : input_methods)
        {
            method_items.push_back(item.c_str());
        }

        int input_method_index = 0;
        for (size_t i = 0; i < input_methods.size(); ++i)
        {
            if (input_methods[i] == config.input_method)
            {
                input_method_index = static_cast<int>(i);
                break;
            }
        }

        if (OverlayUI::ComboRow("鼠标输入方式", &input_method_index, method_items.data(), static_cast<int>(method_items.size())))
        {
            std::string new_input_method = input_methods[input_method_index];

            if (new_input_method != config.input_method)
            {
                config.input_method = new_input_method;
                OverlayConfig_MarkDirty();
                input_method_changed.store(true);
            }
        }

        if (config.input_method == "ARDUINO" || config.input_method == "TEENSY41")
        {
            if (arduinoSerial)
            {
                if (arduinoSerial->isOpen())
                {
                    ImGui::TextColored(ImVec4(0, 255, 0, 255), config.input_method == "TEENSY41" ? "Teensy 4.1 已连接" : "Arduino 已连接");
                }
                else
                {
                    ImGui::TextColored(ImVec4(255, 0, 0, 255), config.input_method == "TEENSY41" ? "Teensy 4.1 未连接" : "Arduino 未连接");
                }
            }

            std::vector<std::string> port_list;
            for (int i = 1; i <= 30; ++i)
            {
                port_list.push_back("COM" + std::to_string(i));
            }

            std::vector<const char*> port_items;
            port_items.reserve(port_list.size());
            for (const auto& port : port_list)
            {
                port_items.push_back(port.c_str());
            }

            int port_index = 0;
            for (size_t i = 0; i < port_list.size(); ++i)
            {
                if (port_list[i] == config.arduino_port)
                {
                    port_index = static_cast<int>(i);
                    break;
                }
            }

            if (OverlayUI::ComboRow(config.input_method == "TEENSY41" ? "Teensy 端口" : "Arduino 端口", &port_index, port_items.data(), static_cast<int>(port_items.size())))
            {
                config.arduino_port = port_list[port_index];
                OverlayConfig_MarkDirty();
                input_method_changed.store(true);
            }

            std::vector<int> baud_rate_list = { 9600, 19200, 38400, 57600, 115200 };
            std::vector<std::string> baud_rate_str_list;
            for (const auto& rate : baud_rate_list)
            {
                baud_rate_str_list.push_back(std::to_string(rate));
            }

            std::vector<const char*> baud_rate_items;
            baud_rate_items.reserve(baud_rate_str_list.size());
            for (const auto& rate_str : baud_rate_str_list)
            {
                baud_rate_items.push_back(rate_str.c_str());
            }

            int baud_rate_index = 0;
            for (size_t i = 0; i < baud_rate_list.size(); ++i)
            {
                if (baud_rate_list[i] == config.arduino_baudrate)
                {
                    baud_rate_index = static_cast<int>(i);
                    break;
                }
            }

            if (OverlayUI::ComboRow(config.input_method == "TEENSY41" ? "Teensy 波特率" : "Arduino 波特率", &baud_rate_index, baud_rate_items.data(), static_cast<int>(baud_rate_items.size())))
            {
                config.arduino_baudrate = baud_rate_list[baud_rate_index];
                OverlayConfig_MarkDirty();
                input_method_changed.store(true);
            }

            if (config.input_method == "TEENSY41")
            {
                ImGui::TextDisabled("Uses the Teensy 4.1 serial mouse bridge protocol.");
            }
            else
            {
                if (OverlayUI::CheckboxRow("Arduino 16位鼠标", &config.arduino_16_bit_mouse))
                {
                    OverlayConfig_MarkDirty();
                    input_method_changed.store(true);
                }
                if (OverlayUI::CheckboxRow("Arduino 启用按键", &config.arduino_enable_keys))
                {
                    OverlayConfig_MarkDirty();
                    input_method_changed.store(true);
                }
            }
        }
        else if (config.input_method == "RP2350")
        {
            if (rp2350Serial)
            {
                if (rp2350Serial->isOpen())
                {
                    ImGui::TextColored(ImVec4(0, 255, 0, 255), "RP2350 已连接");
                }
                else
                {
                    ImGui::TextColored(ImVec4(255, 0, 0, 255), "RP2350 未连接");
                }
            }

            std::vector<std::string> port_list;
            for (int i = 1; i <= 30; ++i)
            {
                port_list.push_back("COM" + std::to_string(i));
            }

            std::vector<const char*> port_items;
            port_items.reserve(port_list.size());
            for (const auto& port : port_list)
            {
                port_items.push_back(port.c_str());
            }

            int port_index = 0;
            for (size_t i = 0; i < port_list.size(); ++i)
            {
                if (port_list[i] == config.rp2350_port)
                {
                    port_index = static_cast<int>(i);
                    break;
                }
            }

            if (OverlayUI::ComboRow("RP2350 端口", &port_index, port_items.data(), static_cast<int>(port_items.size())))
            {
                config.rp2350_port = port_list[port_index];
                OverlayConfig_MarkDirty();
                input_method_changed.store(true);
            }

            std::vector<int> baud_rate_list = { 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600 };
            std::vector<std::string> baud_rate_str_list;
            for (const auto& rate : baud_rate_list)
            {
                baud_rate_str_list.push_back(std::to_string(rate));
            }

            std::vector<const char*> baud_rate_items;
            baud_rate_items.reserve(baud_rate_str_list.size());
            for (const auto& rate_str : baud_rate_str_list)
            {
                baud_rate_items.push_back(rate_str.c_str());
            }

            int baud_rate_index = 0;
            for (size_t i = 0; i < baud_rate_list.size(); ++i)
            {
                if (baud_rate_list[i] == config.rp2350_baudrate)
                {
                    baud_rate_index = static_cast<int>(i);
                    break;
                }
            }

            if (OverlayUI::ComboRow("RP2350 波特率", &baud_rate_index, baud_rate_items.data(), static_cast<int>(baud_rate_items.size())))
            {
                config.rp2350_baudrate = baud_rate_list[baud_rate_index];
                OverlayConfig_MarkDirty();
                input_method_changed.store(true);
            }

            if (OverlayUI::CheckboxRow("RP2350 16位鼠标", &config.rp2350_16_bit_mouse))
            {
                OverlayConfig_MarkDirty();
                input_method_changed.store(true);
            }
            if (OverlayUI::CheckboxRow("RP2350 启用按键", &config.rp2350_enable_keys))
            {
                OverlayConfig_MarkDirty();
                input_method_changed.store(true);
            }
        }
        else if (config.input_method == "GHUB")
        {
            if (ghub_version == "13.1.4")
            {
                std::string ghub_version_label = "GHub 正确版本已安装：" + ghub_version;
                ImGui::Text(ghub_version_label.c_str());
            }
            else
            {
                ImGui::Text("GHub 版本不正确或默认路径未设置\n默认系统路径：C:\\Program Files\\LGHUB");
                if (OverlayUI::ButtonRow("GHub", "打开 GHub 文档", "ghub_docs"))
                {
                    ShellExecute(0, 0, L"https://github.com/SunOner/sunone_aimbot_2/blob/main/docs/guides.md#g-hub-input-method", 0, 0, SW_SHOW);
                }
            }

            ImGui::TextColored(ImVec4(255, 0, 0, 255), "风险自负，此方法在某些游戏中被检测");
        }
        else if (config.input_method == "TEENSY41_HID")
        {
            bool teensy41Connected = false;
            {
                std::lock_guard<std::mutex> lock(inputDevicesMutex);
                teensy41Connected = activeMouseInputOwner && activeMouseInputOwner->isOpen();
            }

            if (teensy41Connected)
            {
                ImGui::TextColored(ImVec4(0, 255, 0, 255), "Teensy 4.1 RawHID 已连接");
            }
            else
            {
                ImGui::TextColored(ImVec4(255, 0, 0, 255), "Teensy 4.1 RawHID 未连接");
            }

            static char serial[64] = "";
            static char vid[16] = "";
            static char pid[16] = "";
            static std::string last_serial;
            static std::string last_vid;
            static std::string last_pid;
            static int usage_page = 0;
            static int usage_id = 0;
            static int open_index = 0;
            static int timeout_ms = 0;
            static int reconnect_ms = 0;

            if (last_serial != config.teensy_hid_serial ||
                last_vid != config.teensy_hid_vid_filter ||
                last_pid != config.teensy_hid_pid_filter ||
                usage_page != config.teensy_hid_usage_page ||
                usage_id != config.teensy_hid_usage_id ||
                open_index != config.teensy_hid_open_index ||
                timeout_ms != config.teensy_hid_packet_timeout_ms ||
                reconnect_ms != config.teensy_hid_reconnect_interval_ms)
            {
                strncpy(serial, config.teensy_hid_serial.c_str(), sizeof(serial));
                strncpy(vid, config.teensy_hid_vid_filter.c_str(), sizeof(vid));
                strncpy(pid, config.teensy_hid_pid_filter.c_str(), sizeof(pid));
                serial[sizeof(serial) - 1] = '\0';
                vid[sizeof(vid) - 1] = '\0';
                pid[sizeof(pid) - 1] = '\0';
                last_serial = config.teensy_hid_serial;
                last_vid = config.teensy_hid_vid_filter;
                last_pid = config.teensy_hid_pid_filter;
                usage_page = config.teensy_hid_usage_page;
                usage_id = config.teensy_hid_usage_id;
                open_index = config.teensy_hid_open_index;
                timeout_ms = config.teensy_hid_packet_timeout_ms;
                reconnect_ms = config.teensy_hid_reconnect_interval_ms;
            }

            OverlayUI::InputTextRow("序列号", serial, sizeof(serial));
            OverlayUI::InputTextRow("VID 过滤", vid, sizeof(vid));
            OverlayUI::InputTextRow("PID 过滤", pid, sizeof(pid));
            OverlayUI::InputIntRow("用途页", &usage_page);
            OverlayUI::InputIntRow("用途 ID", &usage_id);
            OverlayUI::InputIntRow("打开索引", &open_index);
            OverlayUI::InputIntRow("数据包超时 ms", &timeout_ms);
            OverlayUI::InputIntRow("重连间隔 ms", &reconnect_ms);

            if (OverlayUI::ButtonRow("Teensy HID", "保存并重连", "teensy_hid_save_reconnect"))
            {
                config.teensy_hid_serial = serial;
                config.teensy_hid_vid_filter = vid;
                config.teensy_hid_pid_filter = pid;
                config.teensy_hid_usage_page = usage_page;
                config.teensy_hid_usage_id = usage_id;
                config.teensy_hid_open_index = open_index;
                config.teensy_hid_packet_timeout_ms = timeout_ms;
                config.teensy_hid_reconnect_interval_ms = reconnect_ms;
                last_serial = config.teensy_hid_serial;
                last_vid = config.teensy_hid_vid_filter;
                last_pid = config.teensy_hid_pid_filter;
                OverlayConfig_MarkDirty();
                input_method_changed.store(true);
            }
        }
        else if (config.input_method == "RAZER")
        {
            if (razerControl && razerControl->isOpen())
            {
                ImGui::TextColored(ImVec4(0, 255, 0, 255), "Razer rzctl 已连接");
            }
            else
            {
                ImGui::TextColored(ImVec4(255, 0, 0, 255), "Razer rzctl 未连接");
            }
            ImGui::Text("需要 rzctl.dll 放在 ai.exe 旁边");
            ImGui::TextColored(ImVec4(255, 0, 0, 255), "风险自负，此方法在某些游戏中被检测");
        }
        else if (config.input_method == "WIN32")
        {
            ImGui::TextColored(ImVec4(255, 255, 255, 255), "标准鼠标输入方式，在大多数游戏中可能不工作。建议使用 GHUB、RAZER、ARDUINO、RP2350、TEENSY41 或 TEENSY41_HID");
            ImGui::TextColored(ImVec4(255, 0, 0, 255), "风险自负，此方法在某些游戏中被检测");
        }
        else if (config.input_method == "KMBOX_NET")
        {
            static char ip[32] = "";
            static char port[8] = "";
            static char uuid[16] = "";
            static std::string last_ip;
            static std::string last_port;
            static std::string last_uuid;

            if (last_ip != config.kmbox_net_ip || last_port != config.kmbox_net_port || last_uuid != config.kmbox_net_uuid)
            {
                strncpy(ip, config.kmbox_net_ip.c_str(), sizeof(ip));
                strncpy(port, config.kmbox_net_port.c_str(), sizeof(port));
                strncpy(uuid, config.kmbox_net_uuid.c_str(), sizeof(uuid));
                ip[sizeof(ip) - 1] = '\0';
                port[sizeof(port) - 1] = '\0';
                uuid[sizeof(uuid) - 1] = '\0';
                last_ip = config.kmbox_net_ip;
                last_port = config.kmbox_net_port;
                last_uuid = config.kmbox_net_uuid;
            }

            OverlayUI::InputTextRow("IP", ip, sizeof(ip));
            OverlayUI::InputTextRow("Port", port, sizeof(port));
            OverlayUI::InputTextRow("UUID", uuid, sizeof(uuid));

            if (OverlayUI::ButtonRow("kmboxNet", "保存并重连", "kmbox_net_save_reconnect"))
            {
                config.kmbox_net_ip = ip;
                config.kmbox_net_port = port;
                config.kmbox_net_uuid = uuid;
                last_ip = config.kmbox_net_ip;
                last_port = config.kmbox_net_port;
                last_uuid = config.kmbox_net_uuid;
                OverlayConfig_MarkDirty();
                input_method_changed.store(true);
            }

            bool kmboxNetConnected = false;
            {
                std::lock_guard<std::mutex> lock(inputDevicesMutex);
                KmboxNetConnection* device =
                    activeMouseInputOwner && std::string(activeMouseInputOwner->name()) == "KMBOX_NET"
                    ? activeMouseInputOwner->kmboxNet()
                    : nullptr;
                kmboxNetConnected = device && device->isOpen();
            }

            if (kmboxNetConnected)
            {
                ImGui::TextColored(ImVec4(0, 255, 0, 255), "kmboxNet 已连接");
            }
            else
            {
                ImGui::TextColored(ImVec4(255, 0, 0, 255), "kmboxNet 未连接");
            }

            if (!kmboxNetConnected)
                ImGui::BeginDisabled();

            if (OverlayUI::ButtonRow("kmboxNet box", "重启设备", "kmbox_net_reboot"))
            {
                std::lock_guard<std::mutex> lock(inputDevicesMutex);
                KmboxNetConnection* device =
                    activeMouseInputOwner && std::string(activeMouseInputOwner->name()) == "KMBOX_NET"
                    ? activeMouseInputOwner->kmboxNet()
                    : nullptr;
                if (device && device->isOpen())
                    device->reboot();
            }

            if (OverlayUI::ButtonRow("kmboxNet image", "更换图片", "kmbox_net_image"))
            {
                std::lock_guard<std::mutex> lock(inputDevicesMutex);
                KmboxNetConnection* device =
                    activeMouseInputOwner && std::string(activeMouseInputOwner->name()) == "KMBOX_NET"
                    ? activeMouseInputOwner->kmboxNet()
                    : nullptr;
                if (device && device->isOpen())
                {
                    device->lcdColor(0);
                    device->lcdPicture(gImage_128x160);
                }
            }

            if (!kmboxNetConnected)
                ImGui::EndDisabled();
        }
        else if (config.input_method == "KMBOX_A")
        {
            static char pidvid[32] = "";
            static std::string last_pidvid;

            if (last_pidvid != config.kmbox_a_pidvid)
            {
                strncpy(pidvid, config.kmbox_a_pidvid.c_str(), sizeof(pidvid));
                pidvid[sizeof(pidvid) - 1] = '\0';
                last_pidvid = config.kmbox_a_pidvid;
            }

            OverlayUI::InputTextRow("PIDVID", pidvid, sizeof(pidvid));
            ImGui::TextDisabled("格式：PPPPVVVV（一个字段）");

            if (OverlayUI::ButtonRow("kmboxA", "保存并重连", "kmbox_a_save_reconnect"))
            {
                config.kmbox_a_pidvid = pidvid;
                last_pidvid = config.kmbox_a_pidvid;
                OverlayConfig_MarkDirty();
                input_method_changed.store(true);
            }

            if (kmboxASerial && kmboxASerial->isOpen())
            {
                ImGui::TextColored(ImVec4(0, 255, 0, 255), "kmboxA 已连接");
            }
            else
            {
                ImGui::TextColored(ImVec4(255, 0, 0, 255), "kmboxA 未连接");
            }
        }
        else if (config.input_method == "MAKCU")
        {
            std::vector<std::string> port_list;
            for (int i = 1; i <= 30; ++i)
            {
                port_list.push_back("COM" + std::to_string(i));
            }

            std::vector<const char*> port_items;
            port_items.reserve(port_list.size());
            for (const auto& port : port_list)
            {
                port_items.push_back(port.c_str());
            }

            int port_index = 0;
            for (size_t i = 0; i < port_list.size(); ++i)
            {
                if (port_list[i] == config.makcu_port)
                {
                    port_index = static_cast<int>(i);
                    break;
                }
            }

            if (OverlayUI::ComboRow("Makcu 端口", &port_index, port_items.data(), static_cast<int>(port_items.size())))
            {
                config.makcu_port = port_list[port_index];
                OverlayConfig_MarkDirty();
                input_method_changed.store(true);
            }

            std::vector<int> baud_list = { 9600, 19200, 38400, 57600, 115200 };
            std::vector<std::string> baud_str_list;
            for (int b : baud_list) baud_str_list.push_back(std::to_string(b));

            std::vector<const char*> baud_items;
            baud_items.reserve(baud_list.size());
            for (const auto& baud : baud_str_list)
            {
                baud_items.push_back(baud.c_str());
            }

            int baud_index = 0;
            for (size_t i = 0; i < baud_list.size(); ++i)
            {
                if (baud_list[i] == config.makcu_baudrate)
                {
                    baud_index = static_cast<int>(i);
                    break;
                }
            }

            if (OverlayUI::ComboRow("Makcu 波特率", &baud_index, baud_items.data(), static_cast<int>(baud_items.size())))
            {
                config.makcu_baudrate = baud_list[baud_index];
                OverlayConfig_MarkDirty();
                input_method_changed.store(true);
            }

            if (makcuSerial && makcuSerial->isOpen())
            {
                ImGui::TextColored(ImVec4(0, 255, 0, 255), "Makcu 已连接");
            }
            else
            {
                ImGui::TextColored(ImVec4(255, 0, 0, 255), "Makcu 未连接");
            }
        }

        OverlayUI::EndSection();
    }

    if (prev_fovX != config.fovX ||
        prev_fovY != config.fovY ||
        prev_minSpeedMultiplier != config.minSpeedMultiplier ||
        prev_maxSpeedMultiplier != config.maxSpeedMultiplier ||
        prev_predictionInterval != config.predictionInterval ||
        prev_kalman_enabled != config.kalman_enabled ||
        prev_kalman_process_noise_position != config.kalman_process_noise_position ||
        prev_kalman_process_noise_velocity != config.kalman_process_noise_velocity ||
        prev_kalman_measurement_noise != config.kalman_measurement_noise ||
        prev_kalman_velocity_damping != config.kalman_velocity_damping ||
        prev_kalman_max_velocity != config.kalman_max_velocity ||
        prev_kalman_warmup_frames != config.kalman_warmup_frames ||
        prev_kalman_compensate_detection_delay != config.kalman_compensate_detection_delay ||
        prev_kalman_additional_prediction_ms != config.kalman_additional_prediction_ms ||
        prev_kalman_reset_timeout_sec != config.kalman_reset_timeout_sec ||
        prev_snapRadius != config.snapRadius ||
        prev_nearRadius != config.nearRadius ||
        prev_speedCurveExponent != config.speedCurveExponent ||
        prev_snapBoostFactor != config.snapBoostFactor)
    {
        prev_fovX = config.fovX;
        prev_fovY = config.fovY;
        prev_minSpeedMultiplier = config.minSpeedMultiplier;
        prev_maxSpeedMultiplier = config.maxSpeedMultiplier;
        prev_predictionInterval = config.predictionInterval;
        prev_kalman_enabled = config.kalman_enabled;
        prev_kalman_process_noise_position = config.kalman_process_noise_position;
        prev_kalman_process_noise_velocity = config.kalman_process_noise_velocity;
        prev_kalman_measurement_noise = config.kalman_measurement_noise;
        prev_kalman_velocity_damping = config.kalman_velocity_damping;
        prev_kalman_max_velocity = config.kalman_max_velocity;
        prev_kalman_warmup_frames = config.kalman_warmup_frames;
        prev_kalman_compensate_detection_delay = config.kalman_compensate_detection_delay;
        prev_kalman_additional_prediction_ms = config.kalman_additional_prediction_ms;
        prev_kalman_reset_timeout_sec = config.kalman_reset_timeout_sec;
        prev_snapRadius = config.snapRadius;
        prev_nearRadius = config.nearRadius;
        prev_speedCurveExponent = config.speedCurveExponent;
        prev_snapBoostFactor = config.snapBoostFactor;

        globalMouseThread->updateConfig(
            config.detection_resolution,
            config.fovX,
            config.fovY,
            config.minSpeedMultiplier,
            config.maxSpeedMultiplier,
            config.predictionInterval,
            config.auto_shoot,
            config.bScope_multiplier);

        OverlayConfig_MarkDirty();
    }

    if (prev_wind_mouse_enabled != config.wind_mouse_enabled ||
        prev_wind_G != config.wind_G ||
        prev_wind_W != config.wind_W ||
        prev_wind_M != config.wind_M ||
        prev_wind_D != config.wind_D)
    {
        prev_wind_mouse_enabled = config.wind_mouse_enabled;
        prev_wind_G = config.wind_G;
        prev_wind_W = config.wind_W;
        prev_wind_M = config.wind_M;
        prev_wind_D = config.wind_D;

        globalMouseThread->updateConfig(
            config.detection_resolution,
            config.fovX,
            config.fovY,
            config.minSpeedMultiplier,
            config.maxSpeedMultiplier,
            config.predictionInterval,
            config.auto_shoot,
            config.bScope_multiplier);

        OverlayConfig_MarkDirty();
    }

    if (prev_auto_shoot != config.auto_shoot ||
        prev_bScope_multiplier != config.bScope_multiplier)
    {
        prev_auto_shoot = config.auto_shoot;
        prev_bScope_multiplier = config.bScope_multiplier;

        globalMouseThread->updateConfig(
            config.detection_resolution,
            config.fovX,
            config.fovY,
            config.minSpeedMultiplier,
            config.maxSpeedMultiplier,
            config.predictionInterval,
            config.auto_shoot,
            config.bScope_multiplier);

        OverlayConfig_MarkDirty();
    }
}

void draw_mouse()
{
    draw_mouse_page(MouseSettingsPage::All);
}

void draw_mouse_movement()
{
    draw_mouse_page(MouseSettingsPage::Movement);
}

void draw_mouse_prediction()
{
    draw_mouse_page(MouseSettingsPage::Prediction);
}

void draw_mouse_assist()
{
    draw_mouse_page(MouseSettingsPage::Assist);
}

void draw_mouse_profiles()
{
    draw_mouse_page(MouseSettingsPage::Profiles);
}

void draw_mouse_input()
{
    draw_mouse_page(MouseSettingsPage::Input);
}
