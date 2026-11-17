// BalloonBuster.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//package inclusions
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFPhysics.h>
#include <vector>
#include <SFML/Audio.hpp>


//namespaces
using namespace std;
using namespace sf;
using namespace sfp;

//keyboard speed/movement speed of weapon
const float KB_SPEED = 0.2;

//texture loader
void LoadTex(Texture& tex, string filename) {
    if (!tex.loadFromFile(filename)) {
        cout << "Could not load " << filename << endl;
    }
}

//move crossbow left or right
void MoveCrossbow(PhysicsSprite& crossbow, int elapsedMS) {
    if (Keyboard::isKeyPressed(Keyboard::Right)) {
        Vector2f newPos(crossbow.getCenter());
        newPos.x = newPos.x + (KB_SPEED * elapsedMS);
        crossbow.setCenter(newPos);
    }
    if (Keyboard::isKeyPressed(Keyboard::Left)) {
        Vector2f newPos(crossbow.getCenter());
        newPos.x = newPos.x - (KB_SPEED * elapsedMS);
        crossbow.setCenter(newPos);
    }
}

//text loader
Vector2f GetTextSize(Text text) {
    FloatRect r = text.getGlobalBounds();
    return Vector2f(r.width, r.height);
}

bool playAgain = true;
bool gameOver = false;

//main game loop
int main()
{
    while (!gameOver) {
        //window render

        RenderWindow window(VideoMode(800, 600), "duck hunt");
        World world(Vector2f(0, 0));
        //starting score
        int score(0);
        int arrows(5);

        //i believe that this loads the bubble popping sound
        SoundBuffer popBuffer;
        if (!popBuffer.loadFromFile("balloonpop.ogg")) {
            cout << "could not load balloonpop.ogg" << endl;
            exit(5);
        }
        Sound popSound;
        popSound.setBuffer(popBuffer);

        //this loads the bg music
        Music music;
        if (!music.openFromFile("circus.ogg")) {
            cout << "Failed to load circus.ogg ";
            exit(6);
        }
        music.play();

        //loading the texture of the crossbow, setting its size, and its position
        PhysicsSprite& crossBow = *new PhysicsSprite();
        Texture cbowTex;
        LoadTex(cbowTex, "images/crossbow.png");
        crossBow.setTexture(cbowTex);
        Vector2f sz = crossBow.getSize();
        crossBow.setCenter(Vector2f(400,
            600 - (sz.y / 2)));

        //loading the arrow. defaults to false so that it's not firing
        PhysicsSprite arrow;
        Texture arrowTex;
        LoadTex(arrowTex, "images/arrow.png");
        arrow.setTexture(arrowTex);
        bool drawingArrow(false);

        //border one
        PhysicsRectangle top;
        top.setSize(Vector2f(800, 10));
        top.setCenter(Vector2f(400, 5));
        top.setStatic(true);
        world.AddPhysicsBody(top);


        //border right
        PhysicsRectangle right;
        right.setSize(Vector2f(10, 600));
        right.setCenter(Vector2f(795, 300));
        right.setStatic(true);
        world.AddPhysicsBody(right);


        //loading the balloon texture
        Texture redTex;
        LoadTex(redTex, "images/red_balloon(3).png");
        PhysicsShapeList<PhysicsSprite> balloons;
        //balloon spanwing
        for (int i(0); i < 4; i++) {
            PhysicsSprite& balloon = balloons.Create(); // <-- creates a new instance of the balloon
            balloon.setTexture(redTex);
            balloon.setSize(Vector2f (50,50));
            int x = ((700 / 3) * i);
            Vector2f sz = balloon.getSize();

            //setting parameters and movement of the balloon
            balloon.setCenter(Vector2f(-50 - x, 20 + (sz.y / 2)));
            balloon.setVelocity(Vector2f(0.25, 0));
            world.AddPhysicsBody(balloon);
            //when the balloon is hit by an arrow, it opens an event using one of those funny things we learned the word for in class
            balloon.onCollision =
                [&drawingArrow, &world, &arrow, &balloon, &balloons, &score, &popSound, &right]
                (PhysicsBodyCollisionResult result) {
                if (result.object2 == arrow) {
                    popSound.play();
                    drawingArrow = false;
                    world.RemovePhysicsBody(arrow);
                    world.RemovePhysicsBody(balloon);
                    balloons.QueueRemove(balloon);
                    score += 10;
                }
                if (result.object2 == right) {
                    world.RemovePhysicsBody(balloon);
                    balloons.QueueRemove(balloon);
                }
                };
        }

        //if the arrow hits the top border, the crossbow is reset
        top.onCollision = [&drawingArrow, &world, &arrow]
        (PhysicsBodyCollisionResult result) {
            drawingArrow = false;
            world.RemovePhysicsBody(arrow);
            };

        //loading text and stuff on screen
        Font fnt;
        if (!fnt.loadFromFile("arial.ttf")) {
            cout << "Could not load font." << endl;
            exit(3);
        }

        //some weird clock stuff
        Clock clock;
        Time lastTime(clock.getElapsedTime());
        Time currentTime(lastTime);

        long duckMS(0);
        while ((arrows > 0) || drawingArrow) {
            window.clear();

            currentTime = clock.getElapsedTime();
            Time deltaTime = currentTime - lastTime;
            long deltaMS = deltaTime.asMilliseconds();
            if (deltaMS > 9) {
                duckMS = duckMS + deltaMS;  //increment our duck counter
                lastTime = currentTime;
                world.UpdatePhysics(deltaMS);
                MoveCrossbow(crossBow, deltaMS);
                if (Keyboard::isKeyPressed(Keyboard::Space) &&
                    !drawingArrow) {
                    drawingArrow = true;
                    arrows = arrows - 1;
                    arrow.setCenter(crossBow.getCenter());
                    arrow.setVelocity(Vector2f(0, -1));
                    world.AddPhysicsBody(arrow);
                }
            }
            if (duckMS > 3750) { //2000 MS is 2 seconds
                duckMS = 0;
                for (int i(0); i < 4; i++) {
                    PhysicsSprite& balloon = balloons.Create(); // <-- creates a new instance of the balloon
                    balloon.setTexture(redTex);
                    balloon.setSize(Vector2f(50, 50));
                    int x = ((700 / 3) * i);
                    Vector2f sz = balloon.getSize();
                    //setting parameters and movement of the balloon
                    balloon.setCenter(Vector2f(-50 - x, 20 + (sz.y / 2)));
                    balloon.setVelocity(Vector2f(0.25, 0));
                    world.AddPhysicsBody(balloon);
                    //when the balloon is hit by an arrow, it opens an event using one of those funny things we learned the word for in class
                    balloon.onCollision =
                        [&drawingArrow, &world, &arrow, &balloon, &balloons, &score, &popSound, &right]
                        (PhysicsBodyCollisionResult result) {
                        if (result.object2 == arrow) {
                            popSound.play();
                            drawingArrow = false;
                            world.RemovePhysicsBody(arrow);
                            world.RemovePhysicsBody(balloon);
                            balloons.QueueRemove(balloon);
                            score += 10;
                        }
                        if (result.object2 == right) {
                            world.RemovePhysicsBody(balloon);
                            balloons.QueueRemove(balloon);
                        }
                        };
                }

            }
            balloons.DoRemovals();
            for (PhysicsShape& balloon : balloons) {
                window.draw((PhysicsSprite&)balloon);
            }

            if (drawingArrow) {
                window.draw(arrow);
            }

            //drawing everything on screen again
            window.draw(crossBow);
            Text scoreText;
            scoreText.setString(to_string(score));
            scoreText.setFont(fnt);
            scoreText.setPosition(Vector2f(10, 550));
            window.draw(scoreText);
            Text arrowCountText;
            arrowCountText.setString(to_string(arrows));
            arrowCountText.setFont(fnt);
            arrowCountText.setPosition(Vector2f(790 - GetTextSize(arrowCountText).x, 550));
            window.draw(arrowCountText);
            //world.VisualizeAllBounds(window);

            window.display();
        }

        //shows the last screen when there are no more arrows to fire
        window.display(); // this is needed to see the last frame
        gameOver = true;
        Text gameOverText;
        Text repeat;
        Text YN;
        gameOverText.setString("GAME OVER");
        gameOverText.setFont(fnt);
        repeat.setString("Play Again?");
        repeat.setFont(fnt);
        YN.setString("(Press Y/N)");
        YN.setFont(fnt);
        sz = GetTextSize(gameOverText);
        gameOverText.setPosition(400 - (sz.x / 2), 300 - (sz.y / 2));
        repeat.setPosition(400 - (sz.x / 2), 350 - (sz.y / 2));
        YN.setPosition(400 - (sz.x / 2), 400 - (sz.y / 2));

        window.draw(gameOverText);
        window.draw(repeat);
        window.draw(YN);

        window.display();

        //option to reset game
        while (gameOver) {
            if (Keyboard::isKeyPressed(Keyboard::Y)) {
                score = 0;
                drawingArrow = true;
                arrows = 5;
                gameOver = false;
                window.clear();
                window.display();

            }
            if (Keyboard::isKeyPressed(Keyboard::N)) {
                playAgain = false;
                break;
            }
        }
    }
        while (playAgain);
};

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
