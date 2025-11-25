#ifndef PLOTTER2DELEMENT_HPP
#define PLOTTER2DELEMENT_HPP

#include "BaseElement.hpp"
#include "../../Style.hpp"
#include "../../RenderContext.hpp"
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <glm/glm.hpp>

struct PlotLine {
    std::vector<std::string> xDateStrings;  // original dates
    std::vector<float> xPoints;             // numeric timestamps (days)
    std::vector<float> yPoints;

    std::string label;
    glm::vec4 color;
    float lineWidth = 2.0f;
    
    // Individual scale for this plot line
    float yMin = 0.0f;
    float yMax = 1.0f;
};

class Plotter2dElement : public BaseElement {
private:
    std::vector<PlotLine> plotLines;
    std::string title;
    std::string xAxisLabel;
    std::string yAxisLabel;

    int marginLeft = 60;
    int marginRight = 20;
    int marginTop = 30;
    int marginBottom = 40;

    bool showGrid = true;
    bool showLegend = true;
    bool autoScale = true;

    float xMin = 0.0f, xMax = 1.0f;

    int xGridLines = 5;
    int yGridLines = 5;

    glm::vec4 bgColor;
    glm::vec4 axisColor;
    glm::vec4 gridColor;
    glm::vec4 textColor;

    bool showBackground = false;

public:
    Plotter2dElement(RenderContext* context,
                     int x, int y, int width, int height,
                     const std::string& title = "",
                     int layer = 0,
                     Menu* parent = nullptr);

    ~Plotter2dElement() = default;

    void render() override;
    bool handleEvent(const SDL_Event& event) override;

    // ADD: date-based function
    void addPlotLine(const std::vector<std::string>& dateStrings,
                     const std::vector<float>& yPoints,
                     const std::string& label,
                     const glm::vec4& color,
                     float lineWidth = 2.0f);

    void clearPlotLines();
    void removePlotLine(size_t index);

    // Setters
    void setTitle(const std::string& t) { title = t; }
    void setXAxisLabel(const std::string& label) { xAxisLabel = label; }
    void setYAxisLabel(const std::string& label) { yAxisLabel = label; }
    void setShowGrid(bool show) { showGrid = show; }
    void setShowBackground(bool show) { showBackground = show; }
    void setShowLegend(bool show) { showLegend = show; }
    void setAutoScale(bool auto_scale) { autoScale = auto_scale; }
    void setXRange(float min, float max) { xMin = min; xMax = max; autoScale = false; }
    void setGridLines(int xLines, int yLines) { xGridLines = xLines; yGridLines = yLines; }
    void setMargins(int left, int right, int top, int bottom);

    size_t getPlotLineCount() const { return plotLines.size(); }

private:
    float parseDateToDays(const std::string& date) const;

    void calculateDataRange();
    void calculateIndividualScales();
    float mapX(float dataX) const;
    float mapY(float dataY, const PlotLine& line) const;

    void renderBackground();
    void renderGrid();
    void renderAxes();
    void renderPlotLines();
    void renderLegend();
    void renderTitle();

    glm::vec4 colorToVec4(const SDL_Color& color) const {
        return glm::vec4(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
    }
};

#endif