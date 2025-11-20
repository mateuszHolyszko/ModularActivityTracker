#include "CatWidget.hpp"
#include <chrono>

CatWidget::CatWidget(RenderContext* context,
                     ImageManager* imgManager,
                     int x, int y, int width, int height,
                     int spriteSheetWidth,
                     int spriteSheetHeight,
                     int layer, Menu* parent)
    : SpriteElement(context, imgManager,
                    x, y, width, height,
                    "cat_sprite_sheet",
                    "src/GUI/images/cat/sprites.txt",
                    spriteSheetWidth,
                    spriteSheetHeight,
                    "idle",
                    layer, parent),

      idleTimeDist(0.5f, 1.5f),
      walkTimeDist(1.0f, 2.0f),
      lrDist(0, 1),
      tbDist(0, 1),
      actionPickDist(0, 1),

      state(State::Idle),
      dir(Direction::LB),

      remainingLoops(0),
      lastFrameIndex(0),

      stateTimer(0.0f),
      stateDuration(idleTimeDist(rng))
{
    rng.seed(std::chrono::high_resolution_clock::now()
             .time_since_epoch().count());

    setFrameTime(0.12f);
    play(animIdle(), true);
}

void CatWidget::update(float deltaTime) {

    stateTimer += deltaTime;

    std::string anim = getCurrentAnimationName();
    int frame = getCurrentFrameIndex();
    int totalFrames = getAnimationFrameCount(anim);

    // ---------------------------
    // ACTION STATES (meow/sleep)
    // ---------------------------
    if (state == State::Meowing || state == State::Sleeping) {

        // Detect loop completion: frame wrapped from lastFrameIndex > currentFrame
        if (frame < lastFrameIndex) {
            remainingLoops--;
            if (remainingLoops <= 0) {
                changeState(State::Idle);
            }
        }

        lastFrameIndex = frame;

        SpriteElement::update(deltaTime);
        return;
    }

    // ---------------------------
    // IDLE & WALK based on timers
    // ---------------------------
    if (stateTimer >= stateDuration) {
        switch (state) {
            case State::Idle:
                pickRandomDirection();
                changeState(State::Walking);
                break;

            case State::Walking:
                if (actionPickDist(rng) == 0)
                    changeState(State::Meowing);
                else
                    changeState(State::Sleeping);
                break;

            default:
                break;
        }
    }

    lastFrameIndex = frame;
    SpriteElement::update(deltaTime);
}

void CatWidget::pickRandomDirection() {
    int lr = lrDist(rng); // left/right
    int tb = tbDist(rng); // top/bottom

    if (lr == 0)
        dir = (tb == 0) ? Direction::LB : Direction::LT;
    else
        dir = (tb == 0) ? Direction::RB : Direction::RT;
}

void CatWidget::changeState(State newState) {
    state = newState;
    stateTimer = 0.0f;

    switch (state) {

        case State::Idle:
            stateDuration = idleTimeDist(rng);
            play(animIdle(), true);
            remainingLoops = 0;
            lastFrameIndex = 0;
            break;

        case State::Walking:
            stateDuration = walkTimeDist(rng);
            play(animWalk(dir), true);
            remainingLoops = 0;
            lastFrameIndex = 0;
            break;

        case State::Sleeping: {
            std::uniform_int_distribution<int> loopsDist(2, 5);
            remainingLoops = loopsDist(rng);
            play(animSleep(dir), true);
            lastFrameIndex = 0;
            break;
        }

        case State::Meowing: {
            std::uniform_int_distribution<int> loopsDist(1, 3);
            remainingLoops = loopsDist(rng);
            play(animMeow(dir), true);
            lastFrameIndex = 0;
            break;
        }
    }
}

std::string CatWidget::animWalk(Direction d) const {
    switch (d) {
        case Direction::LB: return "LBwalk";
        case Direction::LT: return "LTwalk";
        case Direction::RT: return "RTwalk";
        case Direction::RB: return "RBwalk";
    }
    return "idle";
}

std::string CatWidget::animSleep(Direction d) const {
    switch (d) {
        case Direction::LB: return "LBsleep";
        case Direction::LT: return "LTsleep";
        case Direction::RT: return "RTsleep";
        case Direction::RB: return "RBsleep";
    }
    return "idle";
}

std::string CatWidget::animMeow(Direction d) const {
    switch (d) {
        case Direction::LB: return "LBmeow";
        case Direction::LT: return "LTmeow";
        case Direction::RT: return "RTmeow";
        case Direction::RB: return "RBmeow";
    }
    return "idle";
}

std::string CatWidget::animIdle() const {
    return "idle";
}
