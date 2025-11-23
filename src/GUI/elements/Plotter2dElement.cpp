#include "Plotter2dElement.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

Plotter2dElement::Plotter2dElement(RenderContext* context,
                                   int x, int y, int width, int height,
                                   const std::string& title,
                                   int layer, Menu* parent)
    : BaseElement(context, x, y, width, height, false, layer, parent),
      title(title)
{
    bgColor = colorToVec4(style.getLgBgColor());
    axisColor = colorToVec4(style.getTextColor());
    gridColor = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
    textColor = colorToVec4(style.getTextColor());
}

float Plotter2dElement::parseDateToDays(const std::string& date) const {
    std::tm tm = {};
    std::istringstream ss(date);
    ss >> std::get_time(&tm, "%Y-%m-%d");

    if (ss.fail())
        return 0.0f;

    std::time_t t = std::mktime(&tm);
    return static_cast<float>(t) / 86400.0f; // days
}

void Plotter2dElement::addPlotLine(const std::vector<std::string>& dateStrings,
                                   const std::vector<float>& yPoints,
                                   const std::string& label,
                                   const glm::vec4& color,
                                   float lineWidth)
{
    PlotLine line;
    line.xDateStrings = dateStrings;
    line.yPoints = yPoints;
    line.label = label;
    line.color = color;
    line.lineWidth = lineWidth;

    line.xPoints.reserve(dateStrings.size());
    for (const auto& d : dateStrings)
        line.xPoints.push_back(parseDateToDays(d));

    plotLines.push_back(line);

    if (autoScale)
        calculateDataRange();
}

void Plotter2dElement::clearPlotLines() {
    plotLines.clear();
    if (autoScale) {
        xMin = 0.0f;
        xMax = 1.0f;
        yMin = 0.0f;
        yMax = 1.0f;
    }
}

void Plotter2dElement::removePlotLine(size_t index) {
    if (index < plotLines.size()) {
        plotLines.erase(plotLines.begin() + index);
        if (autoScale) calculateDataRange();
    }
}

void Plotter2dElement::setMargins(int left, int right, int top, int bottom) {
    marginLeft = left;
    marginRight = right;
    marginTop = top;
    marginBottom = bottom;
}

void Plotter2dElement::calculateDataRange() {
    if (plotLines.empty()) return;

    xMin = std::numeric_limits<float>::max();
    xMax = std::numeric_limits<float>::lowest();
    yMin = std::numeric_limits<float>::max();
    yMax = std::numeric_limits<float>::lowest();

    for (const auto& line : plotLines) {
        for (float v : line.xPoints) {
            xMin = std::min(xMin, v);
            xMax = std::max(xMax, v);
        }
        for (float v : line.yPoints) {
            yMin = std::min(yMin, v);
            yMax = std::max(yMax, v);
        }
    }

    float xPad = (xMax - xMin) * 0.05f;
    float yPad = (yMax - yMin) * 0.05f;
    if (xPad == 0) xPad = 0.5f;
    if (yPad == 0) yPad = 0.5f;

    xMin -= xPad;
    xMax += xPad;
    yMin -= yPad;
    yMax += yPad;
}

float Plotter2dElement::mapX(float dataX) const {
    float plotX = x + marginLeft;
    float plotW = width - marginLeft - marginRight;
    return plotX + (dataX - xMin) / (xMax - xMin) * plotW;
}

float Plotter2dElement::mapY(float dataY) const {
    float plotY = y + marginTop;
    float plotH = height - marginTop - marginBottom;
    return plotY + plotH - (dataY - yMin) / (yMax - yMin) * plotH;
}

void Plotter2dElement::render() {
    if (!visible) return;

    if (showBackground) renderBackground();
    if (showGrid) renderGrid();
    renderAxes();
    renderPlotLines();
    if (!title.empty()) renderTitle();
    if (showLegend && !plotLines.empty()) renderLegend();
}

void Plotter2dElement::renderBackground() {
    GraphicCommand cmd;
    cmd.type = GraphicCommand::BOX;
    cmd.x1 = x;
    cmd.y1 = y;
    cmd.x2 = x + width;
    cmd.y2 = y + height;
    cmd.color = bgColor;
    cmd.layer = layer;
    cmd.filled = true;
    renderContext->graphicQueue.push_back(cmd);
}

void Plotter2dElement::renderGrid() {
    float plotX = x + marginLeft;
    float plotY = y + marginTop;
    float plotW = width - marginLeft - marginRight;
    float plotH = height - marginTop - marginBottom;

    for (int i = 0; i <= xGridLines; ++i) {
        float gx = plotX + (plotW * i / xGridLines);
        GraphicCommand cmd;
        cmd.type = GraphicCommand::LINE;
        cmd.x1 = gx; cmd.y1 = plotY;
        cmd.x2 = gx; cmd.y2 = plotY + plotH;
        cmd.color = gridColor;
        cmd.layer = layer;
        cmd.lineWidth = 1.0f;
        renderContext->graphicQueue.push_back(cmd);
    }

    for (int i = 0; i <= yGridLines; ++i) {
        float gy = plotY + (plotH * i / yGridLines);
        GraphicCommand cmd;
        cmd.type = GraphicCommand::LINE;
        cmd.x1 = plotX; cmd.y1 = gy;
        cmd.x2 = plotX + plotW; cmd.y2 = gy;
        cmd.color = gridColor;
        cmd.layer = layer;
        cmd.lineWidth = 1.0f;
        renderContext->graphicQueue.push_back(cmd);
    }
}

void Plotter2dElement::renderAxes() {
    float plotX = x + marginLeft;
    float plotY = y + marginTop;
    float plotW = width - marginLeft - marginRight;
    float plotH = height - marginTop - marginBottom;

    GraphicCommand xAxis;
    xAxis.type = GraphicCommand::LINE;
    xAxis.x1 = plotX; xAxis.y1 = plotY + plotH;
    xAxis.x2 = plotX + plotW; xAxis.y2 = plotY + plotH;
    xAxis.color = axisColor;
    xAxis.layer = layer + 1;
    xAxis.lineWidth = 2.0f;
    renderContext->graphicQueue.push_back(xAxis);

    GraphicCommand yAxis;
    yAxis.type = GraphicCommand::LINE;
    yAxis.x1 = plotX; yAxis.y1 = plotY;
    yAxis.x2 = plotX; yAxis.y2 = plotY + plotH;
    yAxis.color = axisColor;
    yAxis.layer = layer + 1;
    yAxis.lineWidth = 2.0f;
    renderContext->graphicQueue.push_back(yAxis);

    // NO X LABELS HERE — removed interpolation

    float fontSize = 12.0f;

    for (int i = 0; i <= yGridLines; ++i) {
        float val = yMax - (yMax - yMin) * i / yGridLines;

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << val;

        TextCommand txt;
        txt.text = ss.str();
        txt.font = "";
        txt.x = x + 25;
        txt.y = plotY + (plotH * i / yGridLines) + 4;
        txt.color = textColor;
        txt.layer = layer + 1;
        txt.fontSize = fontSize+5;
        renderContext->textQueue.push_back(txt);
    }

    if (!xAxisLabel.empty()) {
        TextCommand txt;
        txt.text = xAxisLabel;
        txt.font = "";
        txt.x = plotX + plotW / 2 - 20;
        txt.y = y + height - 5;
        txt.color = textColor;
        txt.layer = layer + 1;
        txt.fontSize = fontSize;
        renderContext->textQueue.push_back(txt);
    }

    if (!yAxisLabel.empty()) {
        TextCommand txt;
        txt.text = yAxisLabel;
        txt.font = "";
        txt.x = x + 5;
        txt.y = plotY - 15;
        txt.color = textColor;
        txt.layer = layer + 1;
        txt.fontSize = fontSize;
        renderContext->textQueue.push_back(txt);
    }
}

void Plotter2dElement::renderPlotLines() {
    float baseY = mapY(yMin) + 20; // date label Y position

    for (const auto& line : plotLines) {
        size_t n = std::min(line.xPoints.size(), line.yPoints.size());
        if (n < 2) continue;

        for (size_t i = 0; i < n - 1; ++i) {
            GraphicCommand cmd;
            cmd.type = GraphicCommand::LINE;
            cmd.x1 = mapX(line.xPoints[i]);
            cmd.y1 = mapY(line.yPoints[i]);
            cmd.x2 = mapX(line.xPoints[i + 1]);
            cmd.y2 = mapY(line.yPoints[i + 1]);
            cmd.color = line.color;
            cmd.layer = layer + 2;
            cmd.lineWidth = line.lineWidth;
            renderContext->graphicQueue.push_back(cmd);
        }

        for (size_t i = 0; i < n; ++i) {
            float px = mapX(line.xPoints[i]);
            float py = mapY(line.yPoints[i]);

            GraphicCommand pt;
            pt.type = GraphicCommand::CIRCLE;
            pt.x1 = px;
            pt.y1 = py;
            pt.x2 = 3.0f;
            pt.color = line.color;
            pt.layer = layer + 3;
            pt.filled = true;
            renderContext->graphicQueue.push_back(pt);

            // --- Draw Y-value label near the point ---
            {
                TextCommand tval;
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(2) << line.yPoints[i];   // format y-value
                tval.text = ss.str();
                tval.font = "";

                tval.x = px - 20;        // a bit to the left of the point
                tval.y = py - 15;        // up of point, TODO if downtrend + 8, if uptrend - 8

                tval.color = textColor;
                tval.layer = layer + 4;
                tval.fontSize = 15.0f;

                renderContext->textQueue.push_back(tval);
            }

            // --- Draw the date label under the point ---
            TextCommand txt;
            txt.text = line.xDateStrings[i];
            txt.font = "";

            // Alternate vertical offset to avoid overlap
            float offset = (i % 2 == 0 ? -4.0f : +4.0f);

            txt.x = px - 20;
            txt.y = baseY + offset;

            txt.color = textColor;
            txt.layer = layer + 4;
            txt.fontSize = 10.0f;
            renderContext->textQueue.push_back(txt);
        }
    }
}

void Plotter2dElement::renderLegend() {
    float legendX = x + width - marginRight - 100;
    float legendY = y + marginTop + 10;
    float lineH = 18.0f;
    float boxW = 100.0f;
    float boxH = plotLines.size() * lineH + 10;

    GraphicCommand bg;
    bg.type = GraphicCommand::BOX;
    bg.x1 = legendX; bg.y1 = legendY;
    bg.x2 = legendX + boxW; bg.y2 = legendY + boxH;
    bg.color = glm::vec4(0.15f, 0.15f, 0.15f, 0.9f);
    bg.layer = layer + 4;
    bg.filled = true;
    renderContext->graphicQueue.push_back(bg);

    for (size_t i = 0; i < plotLines.size(); ++i) {
        float ey = legendY + 5 + i * lineH;

        GraphicCommand ln;
        ln.type = GraphicCommand::LINE;
        ln.x1 = legendX + 5; ln.y1 = ey + 8;
        ln.x2 = legendX + 25; ln.y2 = ey + 8;
        ln.color = plotLines[i].color;
        ln.layer = layer + 5;
        ln.lineWidth = 2.0f;
        renderContext->graphicQueue.push_back(ln);

        TextCommand txt;
        txt.text = plotLines[i].label;
        txt.font = "";
        txt.x = legendX + 30;
        txt.y = ey + 12;
        txt.color = textColor;
        txt.layer = layer + 5;
        txt.fontSize = 12.0f;
        renderContext->textQueue.push_back(txt);
    }
}

void Plotter2dElement::renderTitle() {
    TextCommand txt;
    txt.text = title;
    txt.font = "";
    txt.x = x + width / 2 - title.length() * 4;
    txt.y = y + 18;
    txt.color = textColor;
    txt.layer = layer + 1;
    txt.fontSize = 16.0f;
    renderContext->textQueue.push_back(txt);
}

bool Plotter2dElement::handleEvent(const SDL_Event& event) {
    return false;
}
