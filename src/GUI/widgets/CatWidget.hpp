#ifndef CATWIDGET_HPP
#define CATWIDGET_HPP

#include "../elements/SpriteElement.hpp"
#include <random>

class CatWidget : public SpriteElement {
public:
    enum class Direction { LB, LT, RT, RB };
    enum class State { Idle, Walking, Sleeping, Meowing };

    CatWidget(RenderContext* context,
              ImageManager* imgManager,
              int x, int y, int width, int height,
              int spriteSheetWidth,
              int spriteSheetHeight,
              int layer = 0, Menu* parent = nullptr);

    void update(float deltaTime) override;

private:
    std::mt19937 rng;

    std::uniform_int_distribution<int> lrDist;
    std::uniform_int_distribution<int> tbDist;
    std::uniform_int_distribution<int> actionPickDist;

    State state;
    Direction dir;

    int remainingLoops;
    int lastFrameIndex;

    float singleFrameAccum;  // <-- important for 1-frame idle loop detection

    void changeState(State newState);
    void pickRandomDirection();

    std::string animWalk(Direction d) const;
    std::string animSleep(Direction d) const;
    std::string animMeow(Direction d) const;
    std::string animIdle() const;
};

#endif
