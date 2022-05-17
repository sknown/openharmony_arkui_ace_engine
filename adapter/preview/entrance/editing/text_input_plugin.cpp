/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "adapter/preview/entrance/editing/text_input_plugin.h"

#include <map>
#include "base/log/log.h"

namespace OHOS::Ace::Platform {
namespace {

const std::map<int, KeyAction> ACTION_MAP = {
    {GLFW_RELEASE, KeyAction::UP},
    {GLFW_PRESS, KeyAction::DOWN},
    {GLFW_REPEAT, KeyAction::LONG_PRESS},
};

const std::map<int, KeyCode> CODE_MAP = {
    {GLFW_KEY_BACKSPACE, KeyCode::KEY_FORWARD_DEL},
    {GLFW_KEY_DELETE, KeyCode::KEY_DEL},
    {GLFW_KEY_ESCAPE, KeyCode::KEY_ESCAPE},
    {GLFW_KEY_ENTER, KeyCode::KEY_ENTER},
    {GLFW_KEY_CAPS_LOCK, KeyCode::KEY_CAPS_LOCK},
    {GLFW_KEY_UP, KeyCode::KEY_DPAD_UP},
    {GLFW_KEY_DOWN, KeyCode::KEY_DPAD_DOWN},
    {GLFW_KEY_LEFT, KeyCode::KEY_DPAD_LEFT},
    {GLFW_KEY_RIGHT, KeyCode::KEY_DPAD_RIGHT},
    {GLFW_KEY_GRAVE_ACCENT, KeyCode::KEY_GRAVE},
    {GLFW_KEY_MINUS, KeyCode::KEY_MINUS},
    {GLFW_KEY_EQUAL, KeyCode::KEY_EQUALS},
    {GLFW_KEY_TAB, KeyCode::KEY_TAB},
    {GLFW_KEY_LEFT_BRACKET, KeyCode::KEY_LEFT_BRACKET},
    {GLFW_KEY_RIGHT_BRACKET, KeyCode::KEY_RIGHT_BRACKET},
    {GLFW_KEY_BACKSLASH, KeyCode::KEY_BACKSLASH},
    {GLFW_KEY_SEMICOLON, KeyCode::KEY_SEMICOLON},
    {GLFW_KEY_APOSTROPHE, KeyCode::KEY_APOSTROPHE},
    {GLFW_KEY_COMMA, KeyCode::KEY_COMMA},
    {GLFW_KEY_PERIOD, KeyCode::KEY_PERIOD},
    {GLFW_KEY_SLASH, KeyCode::KEY_SLASH},
    {GLFW_KEY_SPACE, KeyCode::KEY_SPACE},
    {GLFW_KEY_KP_DIVIDE, KeyCode::KEY_NUMPAD_DIVIDE},
    {GLFW_KEY_KP_MULTIPLY, KeyCode::KEY_NUMPAD_MULTIPLY},
    {GLFW_KEY_KP_SUBTRACT, KeyCode::KEY_NUMPAD_SUBTRACT},
    {GLFW_KEY_KP_ADD, KeyCode::KEY_NUMPAD_ADD},
    {GLFW_KEY_KP_ENTER, KeyCode::KEY_NUMPAD_ENTER},
    {GLFW_KEY_KP_EQUAL, KeyCode::KEY_NUMPAD_EQUALS},
    {GLFW_KEY_NUM_LOCK, KeyCode::KEY_NUM_LOCK},
};

}

void TextInputPlugin::KeyboardHook(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (keyboardHookCallback_ && RecognizeKeyEvent(key, action, mods)) {
        keyboardHookCallback_(keyEvent_);
    } else {
        LOGW("Unrecognized key type.");
    }
}

void TextInputPlugin::CharHook(GLFWwindow* window, unsigned int code_point)
{
    if (charHookCallback_) {
        charHookCallback_(code_point);
    }
}

void TextInputPlugin::RegisterKeyboardHookCallback(KeyboardHookCallback&& keyboardHookCallback)
{
    if (keyboardHookCallback) {
        keyboardHookCallback_ = std::move(keyboardHookCallback);
    }
}

void TextInputPlugin::RegisterCharHookCallback(CharHookCallback&& charHookCallback)
{
    if (charHookCallback) {
        charHookCallback_ = std::move(charHookCallback);
    }
}

bool TextInputPlugin::RecognizeKeyEvent(int key, int action, int mods)
{
    auto iterAction = ACTION_MAP.find(action);
    if (iterAction == ACTION_MAP.end()) {
        return false;
    }
    keyEvent_.action = iterAction->second;

    keyEvent_.pressedCodes.clear();
    if (mods & GLFW_MOD_CONTROL) {
        keyEvent_.pressedCodes.push_back(KeyCode::KEY_CTRL_LEFT);
    }
    if (mods & GLFW_MOD_SHIFT) {
        keyEvent_.pressedCodes.push_back(KeyCode::KEY_SHIFT_LEFT);
    }
    if (mods & GLFW_MOD_ALT) {
        keyEvent_.pressedCodes.push_back(KeyCode::KEY_ALT_LEFT);
    }

    auto iterCode = CODE_MAP.find(key);
    if (iterCode == CODE_MAP.end() && !(key >= GLFW_KEY_A && key <= GLFW_KEY_Z) &&
        !(key >= GLFW_KEY_0 && key <= GLFW_KEY_9) && !(key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_9)) {
        return false;
    }
    if (iterCode != CODE_MAP.end()) {
        keyEvent_.code = iterCode->second;
    }
    if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
        keyEvent_.code = static_cast<KeyCode>(static_cast<int32_t>(KeyCode::KEY_A) + key - GLFW_KEY_A);
    }
    if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
        keyEvent_.code = static_cast<KeyCode>(static_cast<int32_t>(KeyCode::KEY_0) + key - GLFW_KEY_0);
    }
    if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_9) {
        keyEvent_.code = static_cast<KeyCode>(static_cast<int32_t>(KeyCode::KEY_0) + key - GLFW_KEY_KP_0);
    }

    keyEvent_.key = KeyToString(static_cast<int32_t>(keyEvent_.code));
    keyEvent_.pressedCodes.push_back(keyEvent_.code);

    return true;
}

} // namespace OHOS::Ace::Platform
