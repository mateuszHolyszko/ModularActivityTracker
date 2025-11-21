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

      lrDist(0, 1),
      tbDist(0, 1),
      actionPickDist(0, 1),

      state(State::Idle),
      dir(Direction::LB),

      remainingLoops(6), // idle = fixed 6 loops
      lastFrameIndex(0),
      singleFrameAccum(0.0f)
{
    rng.seed(std::chrono::high_resolution_clock::now()
             .time_since_epoch().count());

    setFrameTime(0.12f);
    play(animIdle(), true);
}

void CatWidget::update(float deltaTime) {

    std::string anim = getCurrentAnimationName();
    int currentFrame = getCurrentFrameIndex();
    int frameCount = getAnimationFrameCount(anim);

    bool loopCompleted = false;

    // ------------------------------------
    // LOOP DETECTION LOGIC
    // ------------------------------------
    if (frameCount > 1) {
        // Multi-frame animation: loop occurs when frame index resets (wraps)
        if (currentFrame < lastFrameIndex) {
            loopCompleted = true;
        }
    } else {
        // Single-frame animation (idle)
        singleFrameAccum += deltaTime;
        if (singleFrameAccum >= getFrameTime()) {
            singleFrameAccum -= getFrameTime();
            loopCompleted = true;
        }
    }

    // ------------------------------------
    // HANDLE LOOP COMPLETION
    // ------------------------------------
    if (loopCompleted) {
        remainingLoops--;

        if (remainingLoops <= 0) {

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

                case State::Meowing:
                case State::Sleeping:
                    changeState(State::Idle);
                    break;
            }
        }
    }

    lastFrameIndex = currentFrame;

    SpriteElement::update(deltaTime);
}

void CatWidget::pickRandomDirection() {
    int lr = lrDist(rng);
    int tb = tbDist(rng);

    if (lr == 0)
        dir = (tb == 0) ? Direction::LB : Direction::LT;
    else
        dir = (tb == 0) ? Direction::RB : Direction::RT;
}

void CatWidget::changeState(State newState) {
    state = newState;
    singleFrameAccum = 0.0f;

    switch (state) {

        case State::Idle:
            remainingLoops = 6;              // EXACTLY 6 loops
            play(animIdle(), true);
            lastFrameIndex = 0;
            break;

        case State::Walking: {
            std::uniform_int_distribution<int> loops(3, 5);
            remainingLoops = loops(rng);     // 3–5 loops
            play(animWalk(dir), true);
            lastFrameIndex = 0;
            break;
        }

        case State::Sleeping: {
            std::uniform_int_distribution<int> loops(2, 5);
            remainingLoops = loops(rng);     // 2–5 loops
            play(animSleep(dir), true);
            lastFrameIndex = 0;
            break;
        }

        case State::Meowing: {
            std::uniform_int_distribution<int> loops(1, 3);
            remainingLoops = loops(rng);     // 1–3 loops
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
