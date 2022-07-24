#include "widgets.h"

#include <ImGuiColorTextEdit/TextEditor.h>

#include "absl/status/status.h"
#include "app/core/constants.h"

namespace yaze {
namespace gui {
namespace widgets {

static bool BeginCentered(const char *name) {
  ImGuiIO &io = ImGui::GetIO();
  ImVec2 pos(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove |
                           ImGuiWindowFlags_NoDecoration |
                           ImGuiWindowFlags_AlwaysAutoResize;
  return ImGui::Begin(name, nullptr, flags);
}

void DisplayStatus(absl::Status &status) {
  auto title = absl::StrCat("StatusWindow_", status.ToString()).data();
  if (BeginCentered(title)) {
    ImGui::Text(status.ToString().data());
    ImGui::Spacing();
    ImGui::NextColumn();
    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::NewLine();
    ImGui::SameLine(270);
    if (ImGui::Button("OK", ImVec2(200, 0))) {
      status = absl::OkStatus();
    }
    ImGui::End();
  }
}

TextEditor::LanguageDefinition GetAssemblyLanguageDef() {
  TextEditor::LanguageDefinition language_65816;
  for (auto &k : app::core::kKeywords) language_65816.mKeywords.emplace(k);

  for (auto &k : app::core::kIdentifiers) {
    TextEditor::Identifier id;
    id.mDeclaration = "Built-in function";
    language_65816.mIdentifiers.insert(std::make_pair(std::string(k), id));
  }

  language_65816.mTokenRegexStrings.push_back(
      std::make_pair<std::string, TextEditor::PaletteIndex>(
          "[ \\t]*#[ \\t]*[a-zA-Z_]+", TextEditor::PaletteIndex::Preprocessor));
  language_65816.mTokenRegexStrings.push_back(
      std::make_pair<std::string, TextEditor::PaletteIndex>(
          "L?\\\"(\\\\.|[^\\\"])*\\\"", TextEditor::PaletteIndex::String));
  language_65816.mTokenRegexStrings.push_back(
      std::make_pair<std::string, TextEditor::PaletteIndex>(
          "\\'\\\\?[^\\']\\'", TextEditor::PaletteIndex::CharLiteral));
  language_65816.mTokenRegexStrings.push_back(
      std::make_pair<std::string, TextEditor::PaletteIndex>(
          "[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?",
          TextEditor::PaletteIndex::Number));
  language_65816.mTokenRegexStrings.push_back(
      std::make_pair<std::string, TextEditor::PaletteIndex>(
          "[+-]?[0-9]+[Uu]?[lL]?[lL]?", TextEditor::PaletteIndex::Number));
  language_65816.mTokenRegexStrings.push_back(
      std::make_pair<std::string, TextEditor::PaletteIndex>(
          "0[0-7]+[Uu]?[lL]?[lL]?", TextEditor::PaletteIndex::Number));
  language_65816.mTokenRegexStrings.push_back(
      std::make_pair<std::string, TextEditor::PaletteIndex>(
          "0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?",
          TextEditor::PaletteIndex::Number));
  language_65816.mTokenRegexStrings.push_back(
      std::make_pair<std::string, TextEditor::PaletteIndex>(
          "[a-zA-Z_][a-zA-Z0-9_]*", TextEditor::PaletteIndex::Identifier));
  language_65816.mTokenRegexStrings.push_back(
      std::make_pair<std::string, TextEditor::PaletteIndex>(
          "[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/"
          "\\;\\,\\.]",
          TextEditor::PaletteIndex::Punctuation));

  language_65816.mCommentStart = "/*";
  language_65816.mCommentEnd = "*/";
  language_65816.mSingleLineComment = ";";

  language_65816.mCaseSensitive = false;
  language_65816.mAutoIndentation = true;

  language_65816.mName = "65816";

  return language_65816;
}

}  // namespace widgets
}  // namespace gui
}  // namespace yaze
