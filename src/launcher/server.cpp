#include "main.h"
#include "actions.h"
#include "sdl.h"
#include "server.h"
#include "textures.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <sstream>
#include <unordered_map>

namespace
{
    enum fieldType {FIELD_STRING = 0, FIELD_INTEGER, FIELD_OPTIONS, FIELD_CHECKBOX};

    struct fieldOption { std::string value; std::string labelKey; };

    struct serverField
    {
        std::string key;
        std::string labelKey;
        std::string value;
        std::string defaultValue;
        std::string placeholderKey;
        fieldType type;
        int maxLength;
        int step;
        bool commentWhenEmpty;
        std::vector<fieldOption> options;
        SDL_Rect rect = {0, 0, 0, 0};
    };

    const char* serverConfigPath = "config/server-init.cfg";
    bool panelOpen = false;
    bool releaseMouseCapture = false;
    int activeField = -1;

    std::vector<std::string> configLines;
    std::vector<serverField> fields;
    std::array<std::string, 4> descriptionValues;
    std::array<SDL_Rect, 4> descriptionFlagRects = {{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}};

    int selectedDescriptionLanguage = ENGLISH;
    const int padding = 2;
    SDL_Rect panelRect = {padding, padding, SCR_W - (padding * 2), SCR_H - (padding * 2)};
    SDL_Rect launchButtonRect = {0, 0, 0, 0};
    SDL_Rect cancelButtonRect = {0, 0, 0, 0};

    std::string trim(const std::string& value)
    {
        size_t start = value.find_first_not_of(" \t\r\n");
        if(start == std::string::npos) return "";

        size_t end = value.find_last_not_of(" \t\r\n");
        return value.substr(start, end - start + 1);
    }

    bool pointInRect(int x, int y, const SDL_Rect& rect)
    {
        return x >= rect.x && x < rect.x + rect.w && y >= rect.y && y < rect.y + rect.h;
    }

    std::string localize(const std::string& key)
    {
        return getString(key);
    }

    bool isDescriptionField(const serverField& field)
    {
        return field.key == "serverdesc";
    }

    bool isForcedModeField(const serverField& field)
    {
        return field.key == "servforcemode";
    }

    std::array<std::string, 4> defaultDescriptions()
    {
        return {{"Mon serveur Cube Conflict", "My Cube Conflict server", "Мой сервер Cube Conflict", "Mi servidor de Cube Conflict"}};
    }

    std::array<int, 4> descriptionFlagTextures()
    {
        return {{TEX_FRENCH, TEX_ENGLISH, TEX_RUSSIAN, TEX_SPANISH}};
    }

    int clampLanguageId(int language)
    {
        return std::clamp(language, static_cast<int>(FRENCH), static_cast<int>(SPANISH));
    }

    std::array<std::string, 4> splitDescriptions(const std::string& joinedValue)
    {
        std::array<std::string, 4> splitValues = defaultDescriptions();
        size_t start = 0;

        for(int i = 0; i < 4; ++i)
        {
            size_t separator = joinedValue.find('~', start);
            if(separator == std::string::npos)
            {
                splitValues[i] = joinedValue.substr(start);
                for(int j = i + 1; j < 4; ++j) splitValues[j].clear();
                return splitValues;
            }

            splitValues[i] = joinedValue.substr(start, separator - start);
            start = separator + 1;
        }

        return splitValues;
    }

    std::string joinDescriptions()
    {
        return descriptionValues[FRENCH] + "~" + descriptionValues[ENGLISH] + "~" + descriptionValues[RUSSIAN] + "~" + descriptionValues[SPANISH];
    }

    std::string currentDescriptionValue()
    {
        return descriptionValues[clampLanguageId(selectedDescriptionLanguage)];
    }

    std::string& editableFieldValue(serverField& field)
    {
        if(isDescriptionField(field)) return descriptionValues[clampLanguageId(selectedDescriptionLanguage)];
        return field.value;
    }

    SDL_Rect optionLeftRect(const serverField& field)
    {
        const int optionArrowWidth = 24;
        return {field.rect.x, field.rect.y, optionArrowWidth, field.rect.h};
    }

    SDL_Rect optionRightRect(const serverField& field)
    {
        const int optionArrowWidth = 24;
        return {field.rect.x + field.rect.w - optionArrowWidth, field.rect.y, optionArrowWidth, field.rect.h};
    }

    int currentOptionIndex(const serverField& field)
    {
        for(size_t i = 0; i < field.options.size(); ++i)
        {
            if(field.options[i].value == field.value) return static_cast<int>(i);
        }

        return 0;
    }

    std::string optionLabel(const fieldOption& option)
    {
        return localize(option.labelKey);
    }

    void replaceAll(std::string& text, const std::string& from, const std::string& to)
    {
        if(from.empty()) return;

        size_t start = 0;
        while((start = text.find(from, start)) != std::string::npos)
        {
            text.replace(start, from.size(), to);
            start += to.size();
        }
    }

    std::vector<fieldOption> forcedModeOptions()
    {
        std::vector<fieldOption> options =
        {
            {"-1", ""},
            {"1", ""}
        };

        for(int i = 2; i <= 18; ++i) options.push_back({std::to_string(i), ""});
        return options;
    }

    std::string forcedModeLabel(const std::string& value)
    {
        if(value == "-1") return localize("ServerEditor_ForceMode_Disabled");
        if(value == "1") return localize("ServerEditor_ForceMode_CoopEdit");

        int forcedMode = std::atoi(value.c_str());
        if(forcedMode < 2 || forcedMode > 18) return value;

        struct localizedTerm
        {
            const char* key;
        };

        static const std::array<localizedTerm, 4> modes =
        {{
            {"ServerEditor_GameMode_Deathmatch"},
            {"ServerEditor_GameMode_TeamDeathmatch"},
            {"ServerEditor_GameMode_CaptureTheFlag"},
            {"ServerEditor_GameMode_Domination"}
        }};

        static const std::array<localizedTerm, 5> mutators =
        {{
            {"ServerEditor_Mutator_Pickup"},
            {"ServerEditor_Mutator_Random"},
            {"ServerEditor_Mutator_Multiple"},
            {"ServerEditor_Mutator_Identical"},
            {"ServerEditor_Mutator_Regeneration"}
        }};

        int modeIndex = 0;
        int mutatorIndex = 0;
        if(forcedMode == 18)
        {
            modeIndex = 3;
            mutatorIndex = 4;
        }
        else
        {
            int relative = forcedMode - 2;
            modeIndex = relative / 4;
            mutatorIndex = relative % 4;
        }

        std::string label = localize("ServerEditor_ForceMode_Format");
        replaceAll(label, "%MODE%", localize(modes[modeIndex].key));
        replaceAll(label, "%MUTATOR%", localize(mutators[mutatorIndex].key));
        return label;
    }

    std::string unescapeQuoted(const std::string& value)
    {
        std::string unescaped;
        unescaped.reserve(value.size());

        bool escaping = false;
        for(char ch : value)
        {
            if(escaping)
            {
                unescaped.push_back(ch);
                escaping = false;
            }
            else if(ch == '\\') escaping = true;
            else unescaped.push_back(ch);
        }

        if(escaping) unescaped.push_back('\\');
        return unescaped;
    }

    std::string escapeQuoted(const std::string& value)
    {
        std::string escaped;
        escaped.reserve(value.size());

        for(char ch : value)
        {
            if(ch == '\\' || ch == '"') escaped.push_back('\\');
            escaped.push_back(ch);
        }

        return escaped;
    }

    bool parseInteger(const std::string& value, int &parsedValue)
    {
        std::string cleaned = trim(value);
        if(cleaned.empty()) return false;

        errno = 0;
        char *end = nullptr;
        long parsed = std::strtol(cleaned.c_str(), &end, 10);
        if(errno == ERANGE || end == cleaned.c_str() || *end != '\0') return false;
        if(parsed < std::numeric_limits<int>::min() || parsed > std::numeric_limits<int>::max()) return false;

        parsedValue = static_cast<int>(parsed);
        return true;
    }

    void eraseLastUtf8Codepoint(std::string& value)
    {
        while(!value.empty())
        {
            unsigned char ch = static_cast<unsigned char>(value.back());
            value.pop_back();
            if((ch & 0xC0) != 0x80) break;
        }
    }

    void renderText(TTF_Font* font, const std::string& text, int x, int y, unsigned int color, float scale = 1.f, bool centered = false, const SDL_Rect *clipRect = nullptr)
    {
        if(font == nullptr || text.empty()) return;

        RGBA rgba = extractRGBA(color);
        SDL_Color sdlColor = {rgba.r, rgba.g, rgba.b, rgba.a};
        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, text.c_str(), sdlColor);
        if(textSurface == nullptr) return;

        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(sdl::renderer, textSurface);
        if(textTexture != nullptr)
        {
            SDL_Rect textRect = {x, y, static_cast<int>(textSurface->w * scale), static_cast<int>(textSurface->h * scale)};
            if(centered)
            {
                textRect.x -= textRect.w / 2;
                textRect.y -= textRect.h / 2;
            }

            SDL_RenderSetClipRect(sdl::renderer, clipRect);
            SDL_RenderCopy(sdl::renderer, textTexture, nullptr, &textRect);
            SDL_RenderSetClipRect(sdl::renderer, nullptr);
            SDL_DestroyTexture(textTexture);
        }

        SDL_FreeSurface(textSurface);
    }

    void fillRect(const SDL_Rect& rect, unsigned int color)
    {
        RGBA rgba = extractRGBA(color);
        SDL_SetRenderDrawBlendMode(sdl::renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(sdl::renderer, rgba.r, rgba.g, rgba.b, rgba.a);
        SDL_RenderFillRect(sdl::renderer, &rect);
    }

    void drawRect(const SDL_Rect& rect, unsigned int color)
    {
        RGBA rgba = extractRGBA(color);
        SDL_SetRenderDrawBlendMode(sdl::renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(sdl::renderer, rgba.r, rgba.g, rgba.b, rgba.a);
        SDL_RenderDrawRect(sdl::renderer, &rect);
    }

    std::vector<serverField> buildFields()
    {
        return
        {
            {"serverdesc", "ServerEditor_Description", "My Cube Conflict server", "My Cube Conflict server", "ServerEditor_Description_Placeholder", FIELD_STRING, 259, 0, false, {}},
            {"serverip", "ServerEditor_ServerIp", "", "", "ServerEditor_ServerIp_Placeholder", FIELD_STRING, 64, 0, true, {}},
            {"serverport", "ServerEditor_ServerPort", "43000", "43000", "", FIELD_INTEGER, 5, 1, false, {}},
            {"maxclients", "ServerEditor_MaxClients", "20", "20", "", FIELD_INTEGER, 3, 1, false, {}},
            {"gamelength", "ServerEditor_GameLength", "10", "10", "", FIELD_INTEGER, 3, 1, false, {}},
            {"publicserver", "ServerEditor_PublicServer", "0", "0", "", FIELD_OPTIONS, 1, 1, false, {{"0", "ServerEditor_PublicServer_Private"}, {"1", "ServerEditor_PublicServer_Auth"}, {"2", "ServerEditor_PublicServer_Coop"}}},
            {"updatemaster", "ServerEditor_UpdateMaster", "1", "1", "", FIELD_CHECKBOX, 1, 1, false, {}},
            {"adminpass", "ServerEditor_AdminPassword", "PASSWORD123", "PASSWORD123", "", FIELD_STRING, 64, 0, false, {}},
            {"serverpass", "ServerEditor_ServerPassword", "", "", "ServerEditor_ServerPassword_Placeholder", FIELD_STRING, 64, 0, false, {}},
            {"teamkill", "ServerEditor_Teamkill", "1", "1", "", FIELD_CHECKBOX, 1, 1, false, {}},
            {"servaddbots", "ServerEditor_AddBots", "0", "0", "ServerEditor_AddBots_Placeholder", FIELD_INTEGER, 3, 1, false, {}},
            {"servbotminskill", "ServerEditor_BotMinSkill", "60", "60", "", FIELD_INTEGER, 3, 1, false, {}},
            {"servbotmaxskill", "ServerEditor_BotMaxSkill", "90", "90", "", FIELD_INTEGER, 3, 1, false, {}},
            {"servrandommode", "ServerEditor_RandomMode", "0", "0", "", FIELD_CHECKBOX, 1, 1, false, {}},
            {"servforcemode", "ServerEditor_ForceMode", "-1", "-1", "", FIELD_OPTIONS, 3, 1, false, forcedModeOptions()}
        };
    }

    serverField* findField(const std::string& key)
    {
        for(auto& field : fields)
        {
            if(field.key == key) return &field;
        }

        return nullptr;
    }

    const serverField* getField(const std::string& key)
    {
        for(const auto& field : fields)
        {
            if(field.key == key) return &field;
        }

        return nullptr;
    }

    bool isFieldDisabled(const serverField& field)
    {
        if(!isForcedModeField(field)) return false;

        const serverField* randomModeField = getField("servrandommode");
        return randomModeField != nullptr && randomModeField->value != "0";
    }

    void syncDependentFields()
    {
        const serverField* randomModeField = getField("servrandommode");
        serverField* forcedModeField = findField("servforcemode");
        if(randomModeField == nullptr || forcedModeField == nullptr) return;

        if(randomModeField->value != "0") forcedModeField->value = "-1";
    }

    bool parseConfigAssignment(const std::string& line, std::string& key, std::string& value)
    {
        std::string cleaned = trim(line);
        if(cleaned.empty()) return false;
        if(cleaned.rfind("//", 0) == 0 || cleaned[0] == '#') return false;

        std::istringstream stream(cleaned);
        if(!(stream >> key)) return false;

        std::getline(stream, value);
        value = trim(value);
        if(value.empty()) return false;

        if(value.front() == '"' && value.size() >= 2 && value.back() == '"')
        {
            value = unescapeQuoted(value.substr(1, value.size() - 2));
        }

        return true;
    }

    void layout()
    {
        int panelPadding = 24;
        int columnGap = 24;
        int fieldWidth = (panelRect.w - (panelPadding * 2) - columnGap) / 2;
        int fieldHeight = 28;
        int rowStep = 48;
        int descriptionGap = 12;
        int descriptionFlagWidth = 28;
        int descriptionFlagHeight = 21;
        int descriptionFlagSpacing = 6;
        int descriptionFlagsWidth = (descriptionFlagWidth * 4) + (descriptionFlagSpacing * 3);

        if(!fields.empty())
        {
            int descriptionFieldWidth = panelRect.w - (panelPadding * 2) - descriptionGap - descriptionFlagsWidth;
            fields[0].rect = {panelRect.x + panelPadding, panelRect.y + 86, descriptionFieldWidth, fieldHeight};

            int flagsX = fields[0].rect.x + fields[0].rect.w + descriptionGap;
            int flagsY = fields[0].rect.y + ((fieldHeight - descriptionFlagHeight) / 2);
            for(int i = 0; i < 4; ++i)
            {
                descriptionFlagRects[i] = {flagsX + (i * (descriptionFlagWidth + descriptionFlagSpacing)), flagsY, descriptionFlagWidth, descriptionFlagHeight};
            }
        }

        int leftX = panelRect.x + panelPadding;
        int rightX = leftX + fieldWidth + columnGap;
        int baseY = panelRect.y + 152;

        for(size_t i = 1; i < fields.size(); ++i)
        {
            int column = ((i - 1) % 2 == 0) ? 0 : 1;
            int row = static_cast<int>((i - 1) / 2);
            int fieldX = column == 0 ? leftX : rightX;
            int fieldY = baseY + (row * rowStep);
            fields[i].rect = {fieldX, fieldY, fieldWidth, fieldHeight};
        }

        int buttonY = panelRect.y + panelRect.h - 60;
        launchButtonRect = {panelRect.x + panelRect.w - 264, buttonY, 240, 32};
        cancelButtonRect = {panelRect.x + panelRect.w - 424, buttonY, 140, 32};
    }

    std::string fieldDisplayValue(const serverField& field)
    {
        if(isDescriptionField(field)) return currentDescriptionValue();
        if(field.type == FIELD_CHECKBOX) return "";
        if(isForcedModeField(field)) return forcedModeLabel(field.value);

        if(field.type == FIELD_OPTIONS)
        {
            for(const auto& option : field.options)
            {
                if(option.value == field.value) return optionLabel(option);
            }
        }

        return field.value;
    }

    void cycleField(serverField& field, int direction)
    {
        if(isFieldDisabled(field)) return;

        if(field.type == FIELD_CHECKBOX)
        {
            field.value = (field.value == "0") ? "1" : "0";
        }
        else if(field.type == FIELD_OPTIONS && !field.options.empty())
        {
            int index = currentOptionIndex(field);
            index = std::clamp(index + direction, 0, static_cast<int>(field.options.size()) - 1);
            field.value = field.options[index].value;
        }
        else if(field.type == FIELD_INTEGER)
        {
            int currentValue = 0;
            if(!parseInteger(field.value, currentValue)) parseInteger(field.defaultValue, currentValue);
            currentValue += field.step * direction;
            field.value = std::to_string(currentValue);
        }

        syncDependentFields();
    }

    void moveSelection(int direction)
    {
        if(fields.empty()) return;

        int nextField = activeField;
        for(size_t attempt = 0; attempt < fields.size(); ++attempt)
        {
            if(nextField < 0) nextField = 0;
            else
            {
                nextField += direction;
                if(nextField < 0) nextField = static_cast<int>(fields.size()) - 1;
                if(nextField >= static_cast<int>(fields.size())) nextField = 0;
            }

            if(!isFieldDisabled(fields[nextField]))
            {
                activeField = nextField;
                return;
            }
        }

        activeField = -1;
    }

    bool validateIntegerField(const std::string& key, int minValue, int maxValue)
    {
        const serverField* field = getField(key);
        if(field == nullptr) return false;

        int parsedValue = 0;
        if(!parseInteger(field->value, parsedValue))
        {
            error::pop(localize("Error_Title"), localize("ServerEditor_InvalidValue") + " " + localize(field->labelKey));
            return false;
        }

        if(parsedValue < minValue || parsedValue > maxValue)
        {
            error::pop(localize("Error_Title"), localize("ServerEditor_OutOfRange") + " " + localize(field->labelKey));
            return false;
        }

        return true;
    }

    bool validateFields()
    {
        std::string joinedDescriptions = joinDescriptions();
        if(joinedDescriptions.size() > 259)
        {
            error::pop(localize("Error_Title"), localize("ServerEditor_DescriptionTooLong"));
            return false;
        }

        loopi(4)
        {
            if(descriptionValues[i].find('~') != std::string::npos)
            {
                error::pop(localize("Error_Title"), localize("ServerEditor_DescriptionNoTilde"));
                return false;
            }
        }

        if(!validateIntegerField("serverport", 1, 65535)) return false;
        if(!validateIntegerField("maxclients", 1, 128)) return false;
        if(!validateIntegerField("gamelength", 1, 1440)) return false;
        if(!validateIntegerField("servaddbots", 0, 200)) return false;
        if(!validateIntegerField("servbotminskill", 0, 100)) return false;
        if(!validateIntegerField("servbotmaxskill", 0, 100)) return false;
        if(!validateIntegerField("servforcemode", -1, 18)) return false;

        int minSkill = 0;
        int maxSkill = 0;
        parseInteger(getField("servbotminskill")->value, minSkill);
        parseInteger(getField("servbotmaxskill")->value, maxSkill);
        if(minSkill >= maxSkill)
        {
            error::pop(localize("Error_Title"), localize("ServerEditor_BotSkillOrder"));
            return false;
        }

        return true;
    }

    std::string formatFieldLine(const serverField& field)
    {
        if(isDescriptionField(field))
        {
            return field.key + " \"" + escapeQuoted(joinDescriptions()) + "\"";
        }

        if(field.type == FIELD_STRING)
        {
            return field.key + " \"" + escapeQuoted(field.value) + "\"";
        }

        return field.key + " " + trim(field.value);
    }

    void loadConfig()
    {
        fields = buildFields();
        configLines.clear();
        descriptionValues = defaultDescriptions();

        std::ifstream configFile(serverConfigPath);
        if(!configFile.is_open())
        {
            error::pop(localize("Error_Title"), localize("ServerEditor_OpenFailed"));
            return;
        }

        std::string line;
        while(std::getline(configFile, line))
        {
            configLines.push_back(line);

            std::string key, value;
            if(!parseConfigAssignment(line, key, value)) continue;

            serverField* field = findField(key);
            if(field != nullptr)
            {
                if(isDescriptionField(*field)) descriptionValues = splitDescriptions(value);
                else field->value = value;
            }
        }

        syncDependentFields();
    }

    bool saveConfig()
    {
        syncDependentFields();
        if(!validateFields()) return false;

        std::vector<std::string> output = configLines;
        std::unordered_map<std::string, int> lineIndices;

        for(size_t i = 0; i < output.size(); ++i)
        {
            std::string key, value;
            if(parseConfigAssignment(output[i], key, value) && findField(key) != nullptr)
            {
                lineIndices[key] = static_cast<int>(i);
            }
        }

        for(const auto& field : fields)
        {
            if(field.commentWhenEmpty && trim(field.value).empty())
            {
                auto it = lineIndices.find(field.key);
                if(it != lineIndices.end()) output[it->second] = "// " + field.key + " 0.0.0.0";
                continue;
            }

            std::string formattedLine = formatFieldLine(field);
            auto it = lineIndices.find(field.key);
            if(it != lineIndices.end()) output[it->second] = formattedLine;
            else output.push_back(formattedLine);
        }

        std::ofstream configFile(serverConfigPath);
        if(!configFile.is_open())
        {
            error::pop(localize("Error_Title"), localize("ServerEditor_SaveFailed"));
            return false;
        }

        for(size_t i = 0; i < output.size(); ++i)
        {
            configFile << output[i];
            if(i + 1 < output.size()) configFile << "\n";
        }

        configLines = output;
        return true;
    }

    void renderField(const serverField& field, int index)
    {
        bool disabled = isFieldDisabled(field);
        bool hovered = !disabled && pointInRect(mouseX, mouseY, field.rect);
        bool selected = !disabled && index == activeField;

        SDL_Rect labelRect = {field.rect.x, field.rect.y - 20, field.rect.w, 16};
        renderText(sdl::fontTiny, localize(field.labelKey), labelRect.x, labelRect.y, disabled ? 0x8A8A8AFF : 0x303030FF, 1.f, false, &labelRect);

        fillRect(field.rect, disabled ? 0xEAEAEAFF : (selected ? 0xF6F1DDFF : 0xF1F1F1FF));
        drawRect(field.rect, disabled ? 0xC8C8C8FF : (selected ? 0xD79E2DFF : (hovered ? 0x909090FF : 0xB8B8B8FF)));

        if(field.type == FIELD_CHECKBOX)
        {
            int checkSize = std::max(18, field.rect.h - 8);
            int checkX = field.rect.x + (field.rect.w - checkSize) / 2;
            int checkY = field.rect.y + (field.rect.h - checkSize) / 2;
            texture::render(field.value == "0" ? TEX_OFF : TEX_ON, checkX, checkY, checkSize, checkSize, TEX_ALPHA, 0, disabled ? 0xC8C8C8FF : 0xFFFFFFFF);
        }

        std::string displayValue = fieldDisplayValue(field);
        unsigned int textColor = disabled ? 0x8A8A8AFF : 0x252525FF;
        if(displayValue.empty())
        {
            displayValue = localize(field.placeholderKey);
            textColor = disabled ? 0x9A9A9AFF : 0x808080FF;
        }
        else if(selected && (field.type == FIELD_STRING || field.type == FIELD_INTEGER) && ((SDL_GetTicks() / 500) % 2 == 0))
        {
            displayValue += "|";
        }

        if(field.type != FIELD_CHECKBOX)
        {
            if(field.type == FIELD_OPTIONS)
            {
                SDL_Rect leftRect = optionLeftRect(field);
                SDL_Rect rightRect = optionRightRect(field);
                int optionIndex = currentOptionIndex(field);
                bool canDecrement = !disabled && optionIndex > 0;
                bool canIncrement = !disabled && optionIndex + 1 < static_cast<int>(field.options.size());

                renderText(sdl::fontTiny, "<", leftRect.x + (leftRect.w / 2), leftRect.y + (leftRect.h / 2), canDecrement ? 0x666666FF : 0xAEAEAEFF, 1.f, true);
                renderText(sdl::fontTiny, ">", rightRect.x + (rightRect.w / 2), rightRect.y + (rightRect.h / 2), canIncrement ? 0x666666FF : 0xAEAEAEFF, 1.f, true);

                SDL_Rect valueClip = {leftRect.x + leftRect.w + 6, field.rect.y + 4, field.rect.w - leftRect.w - rightRect.w - 12, field.rect.h - 8};
                renderText(sdl::fontTiny, displayValue, field.rect.x + (field.rect.w / 2), field.rect.y + (field.rect.h / 2), textColor, 1.f, true, &valueClip);
            }
            else
            {
                SDL_Rect valueClip = {field.rect.x + 8, field.rect.y + 4, field.rect.w - 16, field.rect.h - 8};
                renderText(sdl::fontTiny, displayValue, field.rect.x + 8, field.rect.y + 6, textColor, 1.f, false, &valueClip);
            }
        }

        if(isDescriptionField(field))
        {
            std::array<int, 4> flagTextures = descriptionFlagTextures();
            loopi(4)
            {
                SDL_Rect borderRect = {descriptionFlagRects[i].x - 2, descriptionFlagRects[i].y - 2, descriptionFlagRects[i].w + 4, descriptionFlagRects[i].h + 4};
                bool selectedFlag = (i == clampLanguageId(selectedDescriptionLanguage));
                bool hoveredFlag = pointInRect(mouseX, mouseY, descriptionFlagRects[i]);

                fillRect(borderRect, selectedFlag ? 0xF6E1B0FF : (hoveredFlag ? 0xE6E6E6FF : 0xF8F8F8FF));
                drawRect(borderRect, selectedFlag ? 0xD79E2DFF : 0xA8A8A8FF);
                texture::render(flagTextures[i], descriptionFlagRects[i].x, descriptionFlagRects[i].y, descriptionFlagRects[i].w, descriptionFlagRects[i].h, TEX_ALPHA);
            }
        }
    }

    void renderButton(const SDL_Rect& rect, const std::string& text, bool hovered, bool accent)
    {
        unsigned int fill = accent ? (hovered ? 0xC69226FF : 0xD7A03AFF) : (hovered ? 0xD8D8D8FF : 0xE6E6E6FF);
        unsigned int border = accent ? 0x895E13FF : 0x9B9B9BFF;
        unsigned int textColor = accent ? 0xFFF8E6FF : 0x303030FF;

        fillRect(rect, fill);
        drawRect(rect, border);
        renderText(sdl::fontTiny, text, rect.x + (rect.w / 2), rect.y + (rect.h / 2), textColor, 1.f, true);
    }
}

namespace serverui
{
    bool isOpen() { return panelOpen; }
    bool blocksLauncherInput() { return panelOpen || releaseMouseCapture; }

    void open()
    {
        selectedDescriptionLanguage = clampLanguageId(config::get(CONF_LANGUAGE));
        loadConfig();
        layout();
        panelOpen = true;
        activeField = fields.empty() ? -1 : 0;
        SDL_StartTextInput();
    }

    void close()
    {
        panelOpen = false;
        activeField = -1;
        SDL_StopTextInput();
    }

    void render()
    {
        if(!panelOpen) return;

        layout();

        SDL_Rect overlayRect = {0, 0, SCR_W, SCR_H};
        fillRect(overlayRect, 0x11111188);
        fillRect(panelRect, 0xFBFBF4FF);
        drawRect(panelRect, 0x575757FF);

        renderText(sdl::fontMain, localize("ServerEditor_Title"), panelRect.x + 24, panelRect.y + 16, 0x252525FF, 0.48f);
        renderText(sdl::fontTiny, localize("ServerEditor_Subtitle"), panelRect.x + 24, panelRect.y + 44, 0x505050FF);

        for(size_t i = 0; i < fields.size(); ++i) renderField(fields[i], static_cast<int>(i));

        renderText(sdl::fontTiny, localize("ServerEditor_Help"), panelRect.x + 24, panelRect.y + panelRect.h - 22, 0x5A5A5AFF);

        renderButton(cancelButtonRect, localize("ServerEditor_Cancel"), pointInRect(mouseX, mouseY, cancelButtonRect), false);
        renderButton(launchButtonRect, localize("ServerEditor_Launch"), pointInRect(mouseX, mouseY, launchButtonRect), true);
    }

    void update(SDL_Event &e)
    {
        if(!panelOpen)
        {
            if(releaseMouseCapture && e.type == SDL_MOUSEBUTTONUP && (e.button.button == SDL_BUTTON_LEFT || e.button.button == SDL_BUTTON_RIGHT))
            {
                releaseMouseCapture = false;
            }
            return;
        }

        layout();

        if(e.type == SDL_MOUSEBUTTONDOWN)
        {
            if(e.button.button == SDL_BUTTON_LEFT)
            {
                if(pointInRect(e.button.x, e.button.y, launchButtonRect))
                {
                    if(saveConfig())
                    {
                        releaseMouseCapture = true;
                        close();
                        action::launchGame(true);
                    }
                    return;
                }

                if(pointInRect(e.button.x, e.button.y, cancelButtonRect))
                {
                    releaseMouseCapture = true;
                    close();
                    return;
                }

                loopi(4)
                {
                    if(pointInRect(e.button.x, e.button.y, descriptionFlagRects[i]))
                    {
                        selectedDescriptionLanguage = i;
                        activeField = 0;
                        return;
                    }
                }

                for(size_t i = 0; i < fields.size(); ++i)
                {
                    if(pointInRect(e.button.x, e.button.y, fields[i].rect))
                    {
                        if(isFieldDisabled(fields[i])) return;

                        activeField = static_cast<int>(i);
                        if(fields[i].type == FIELD_OPTIONS)
                        {
                            if(pointInRect(e.button.x, e.button.y, optionLeftRect(fields[i]))) cycleField(fields[i], -1);
                            else if(pointInRect(e.button.x, e.button.y, optionRightRect(fields[i]))) cycleField(fields[i], 1);
                        }
                        else
                        {
                            if(fields[i].type == FIELD_CHECKBOX) cycleField(fields[i], 1);
                        }
                        return;
                    }
                }

                activeField = -1;
            }
            else if(e.button.button == SDL_BUTTON_RIGHT)
            {
                for(size_t i = 0; i < fields.size(); ++i)
                {
                    if(pointInRect(e.button.x, e.button.y, fields[i].rect))
                    {
                        if(isFieldDisabled(fields[i])) return;

                        activeField = static_cast<int>(i);
                        if(fields[i].type == FIELD_OPTIONS)
                        {
                            if(pointInRect(e.button.x, e.button.y, optionLeftRect(fields[i]))) cycleField(fields[i], -1);
                            else if(pointInRect(e.button.x, e.button.y, optionRightRect(fields[i]))) cycleField(fields[i], 1);
                        }
                        else
                        {
                            cycleField(fields[i], -1);
                        }
                        return;
                    }
                }
            }
        }
        else if(e.type == SDL_TEXTINPUT && activeField >= 0 && activeField < static_cast<int>(fields.size()))
        {
            serverField& field = fields[activeField];
            if(field.type != FIELD_STRING && field.type != FIELD_INTEGER) return;
            std::string& editableValue = editableFieldValue(field);

            std::string filteredInput;
            for(char ch : std::string(e.text.text))
            {
                if(ch == '\r' || ch == '\n') continue;
                if(field.type == FIELD_INTEGER)
                {
                    if(std::isdigit(static_cast<unsigned char>(ch)) || (ch == '-' && editableValue.empty())) filteredInput.push_back(ch);
                }
                else if(ch != '~') filteredInput.push_back(ch);
            }

            if(filteredInput.empty()) return;
            if(static_cast<int>(editableValue.size() + filteredInput.size()) <= field.maxLength) editableValue += filteredInput;
        }
        else if(e.type == SDL_KEYDOWN)
        {
            switch(e.key.keysym.sym)
            {
                case SDLK_ESCAPE:
                    close();
                    return;

                case SDLK_TAB:
                    moveSelection((e.key.keysym.mod & KMOD_SHIFT) ? -1 : 1);
                    return;

                case SDLK_UP:
                    moveSelection(-1);
                    return;

                case SDLK_DOWN:
                    moveSelection(1);
                    return;

                case SDLK_LEFT:
                    if(activeField >= 0 && activeField < static_cast<int>(fields.size()) && !isFieldDisabled(fields[activeField])) cycleField(fields[activeField], -1);
                    return;

                case SDLK_RIGHT:
                    if(activeField >= 0 && activeField < static_cast<int>(fields.size()) && !isFieldDisabled(fields[activeField])) cycleField(fields[activeField], 1);
                    return;

                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    if(saveConfig())
                    {
                        close();
                        action::launchGame(true);
                    }
                    return;

                case SDLK_BACKSPACE:
                    if(activeField >= 0 && activeField < static_cast<int>(fields.size()))
                    {
                        serverField& field = fields[activeField];
                        std::string& editableValue = editableFieldValue(field);
                        if((field.type == FIELD_STRING || field.type == FIELD_INTEGER) && !editableValue.empty()) eraseLastUtf8Codepoint(editableValue);
                    }
                    return;
            }
        }
    }
}
