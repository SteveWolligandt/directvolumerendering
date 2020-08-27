#ifndef BEZIER_WIDGET
#define BEZIER_WIDGET
// ImGui Bezier widget. @r-lyeh, public domain
// v1.02: add BezierValue(); comments; usage
// v1.01: out-of-bounds coord snapping; custom border width; spacing; cosmetics
// v1.00: initial version
//
// [ref] http://robnapier.net/faster-bezier
// [ref] http://easings.net/es#easeInSine
//
// Usage:
// {  static float v[] = { 0.390f, 0.575f, 0.565f, 1.000f };
//    ImGui::Bezier( "easeOutSine", v );       // draw
//    float y = ImGui::BezierValue( 0.5f, v ); // x delta in [0..1] range
// }

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include <iostream>

namespace ImGui {
template <int steps>
void bezier_table(ImVec2 P[4], ImVec2 results[steps + 1]) {
  static float C[(steps + 1) * 4], *K = 0;
  if (!K) {
    K = C;
    for (unsigned step = 0; step <= steps; ++step) {
      float t = (float)step / (float)steps;
      C[step * 4 + 0] = (1 - t) * (1 - t) * (1 - t);  // * P0
      C[step * 4 + 1] = 3 * (1 - t) * (1 - t) * t;    // * P1
      C[step * 4 + 2] = 3 * (1 - t) * t * t;          // * P2
      C[step * 4 + 3] = t * t * t;                    // * P3
    }
  }
  for (unsigned step = 0; step <= steps; ++step) {
    ImVec2 point = {K[step * 4 + 0] * P[0].x + K[step * 4 + 1] * P[1].x +
                        K[step * 4 + 2] * P[2].x + K[step * 4 + 3] * P[3].x,
                    K[step * 4 + 0] * P[0].y + K[step * 4 + 1] * P[1].y +
                        K[step * 4 + 2] * P[2].y + K[step * 4 + 3] * P[3].y};
    results[step] = point;
  }
}

float BezierValue(float dt01, float P[4]) {
  enum { STEPS = 256 };
  ImVec2 Q[4] = {{0, 0}, {P[0], P[1]}, {P[2], P[3]}, {1, 1}};
  ImVec2 results[STEPS + 1];
  bezier_table<STEPS>(Q, results);
  return results[(int)((dt01 < 0 ? 0 : dt01 > 1 ? 1 : dt01) * STEPS)].y;
}

int Bezier(const char* label, float P[4]) {
  // visuals
  enum {
    SMOOTHNESS = 64
  };  // curve smoothness: the higher number of segments, the smoother curve
  enum { CURVE_WIDTH = 4 };  // main curved line width
  enum { LINE_WIDTH = 1 };   // handlers: small lines width
  enum { GRAB_RADIUS = 6 };  // handlers: circle radius
  enum { GRAB_BORDER = 2 };  // handlers: circle border width

  const ImGuiStyle& Style = GetStyle();
  const ImGuiIO& IO = GetIO();
  ImDrawList* DrawList = GetWindowDrawList();
  ImGuiWindow* Window = GetCurrentWindow();
  if (Window->SkipItems) return false;

  // header and spacing
  int changed = SliderFloat4(label, P, 0, 1, "%.3f", 1.0f);
  int hovered = IsItemActive() || IsItemHovered();  // IsItemDragged() ?
  Dummy(ImVec2(0, 3));

  // prepare canvas
  const float avail = GetContentRegionAvailWidth();
  const float dim = ImMin(avail, 128.f);
  ImVec2 Canvas(dim, dim);

  ImRect bb(Window->DC.CursorPos, Window->DC.CursorPos + Canvas);
  ItemSize(bb);
  if (!ItemAdd(bb, NULL)) return changed;

  // const ImGuiID id = Window->GetID(label);
  // hovered |= 0 != IsHovered(ImRect(bb.Min, bb.Min + ImVec2(avail,dim)), id);

  RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg, 1), true,
              Style.FrameRounding);

  // background grid
  for (int i = 0; i <= Canvas.x; i += (Canvas.x / 4)) {
    DrawList->AddLine(ImVec2(bb.Min.x + i, bb.Min.y),
                      ImVec2(bb.Min.x + i, bb.Max.y),
                      GetColorU32(ImGuiCol_TextDisabled));
  }
  for (int i = 0; i <= Canvas.y; i += (Canvas.y / 4)) {
    DrawList->AddLine(ImVec2(bb.Min.x, bb.Min.y + i),
                      ImVec2(bb.Max.x, bb.Min.y + i),
                      GetColorU32(ImGuiCol_TextDisabled));
  }

  // eval curve
  ImVec2 Q[4] = {{0, 0}, {P[0], P[1]}, {P[2], P[3]}, {1, 1}};
  ImVec2 results[SMOOTHNESS + 1];
  bezier_table<SMOOTHNESS>(Q, results);

  // control points: 2 lines and 2 circles
  {
    char buf[128];
    sprintf(buf, "0##%s", label);

    // handle grabbers
    for (int i = 0; i < 2; ++i) {
      ImVec2 pos =
          ImVec2(P[i * 2 + 0], 1 - P[i * 2 + 1]) * (bb.Max - bb.Min) + bb.Min;
      SetCursorScreenPos(pos - ImVec2(GRAB_RADIUS, GRAB_RADIUS));
      InvisibleButton((buf[0]++, buf),
                      ImVec2(2 * GRAB_RADIUS, 2 * GRAB_RADIUS));
      if (IsItemActive() || IsItemHovered()) {
        SetTooltip("(%4.3f, %4.3f)", P[i * 2 + 0], P[i * 2 + 1]);
      }
      if (IsItemActive() && IsMouseDragging(0)) {
        P[i * 2 + 0] += GetIO().MouseDelta.x / Canvas.x;
        P[i * 2 + 1] -= GetIO().MouseDelta.y / Canvas.y;
        changed = true;
      }
    }

    if (hovered || changed) DrawList->PushClipRectFullScreen();

    // draw curve
    {
      ImColor color(GetStyle().Colors[ImGuiCol_PlotLines]);
      for (int i = 0; i < SMOOTHNESS; ++i) {
        ImVec2 p = {results[i + 0].x, 1 - results[i + 0].y};
        ImVec2 q = {results[i + 1].x, 1 - results[i + 1].y};
        ImVec2 r(p.x * (bb.Max.x - bb.Min.x) + bb.Min.x,
                 p.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
        ImVec2 s(q.x * (bb.Max.x - bb.Min.x) + bb.Min.x,
                 q.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
        DrawList->AddLine(r, s, color, CURVE_WIDTH);
      }
    }

    // draw lines and grabbers
    float luma = IsItemActive() || IsItemHovered() ? 0.5f : 1.0f;
    ImVec4 pink(1.00f, 0.00f, 0.75f, luma), cyan(0.00f, 0.75f, 1.00f, luma);
    ImVec4 white(GetStyle().Colors[ImGuiCol_Text]);
    ImVec2 p1 = ImVec2(P[0], 1 - P[1]) * (bb.Max - bb.Min) + bb.Min;
    ImVec2 p2 = ImVec2(P[2], 1 - P[3]) * (bb.Max - bb.Min) + bb.Min;
    DrawList->AddLine(ImVec2(bb.Min.x, bb.Max.y), p1, ImColor(white),
                      LINE_WIDTH);
    DrawList->AddLine(ImVec2(bb.Max.x, bb.Min.y), p2, ImColor(white),
                      LINE_WIDTH);
    DrawList->AddCircleFilled(p1, GRAB_RADIUS, ImColor(white));
    DrawList->AddCircleFilled(p1, GRAB_RADIUS - GRAB_BORDER, ImColor(pink));
    DrawList->AddCircleFilled(p2, GRAB_RADIUS, ImColor(white));
    DrawList->AddCircleFilled(p2, GRAB_RADIUS - GRAB_BORDER, ImColor(cyan));

    if (hovered || changed) DrawList->PopClipRect();

    // restore cursor pos
    SetCursorScreenPos(ImVec2(bb.Min.x, bb.Max.y + GRAB_RADIUS));  // :P
  }

  return changed;
}

void ShowBezierDemo() {
  static float v[] = {0.000f, 0.000f, 1.000f, 1.000f};
  Bezier("easeLinear", v);
  std::cerr <<  ImGui::BezierValue( 0.5f, v ) << '\n';
}
}  // namespace ImGui
#endif
