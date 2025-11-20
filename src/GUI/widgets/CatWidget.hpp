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
    // RNG
    std::mt19937 rng;

    // Randomizers
    std::uniform_real_distribution<float> idleTimeDist;
    std::uniform_real_distribution<float> walkTimeDist;
    std::uniform_int_distribution<int> lrDist;
    std::uniform_int_distribution<int> tbDist;
    std::uniform_int_distribution<int> actionPickDist;

    State state;
    Direction dir;

    // Loop tracking for actions
    int remainingLoops;
    int lastFrameIndex;

    // Timer for idle & walking
    float stateTimer;
    float stateDuration;

    // State-changing helpers
    void changeState(State newState);
    void pickRandomDirection();

    // Animation helpers
    std::string animWalk(Direction d) const;
    std::string animSleep(Direction d) const;
    std::string animMeow(Direction d) const;
    std::string animIdle() const;
};

#endif
