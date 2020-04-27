//Including Libraries
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <fstream>
#include <iostream>
//Constants
#define screenWidth 1000
#define screenHeight 650
#define groundTop 590
#define shadowThickness 5

/// Global Struct, contains stuff that are used in more than one scope
struct Global
{
    //if Mouse Left Button Clicked inside the Game
    bool leftMouseBtn = 0;

    //if Game is Paused
    bool GamePaused = 0;

    sf::Vector2f mousePos; //to save current mouse position

    //Sounds
    sf::SoundBuffer btnHoverbufr, btnClickbufr;
    sf::Sound btnHover, btnClick;
    bool soundEnabled = 1;

    //Cursor
    sf::Texture cursorTexture;
    sf::Sprite cursor;

    //Background
    sf::Texture backgroundTexture;
    sf::Sprite background;

    //Fonts
    sf::Font BtnFont;

    void create()
    {
        //Load Sounds
        btnHoverbufr.loadFromFile("Data/Sounds/btnHover.wav");
        btnHover.setBuffer(btnHoverbufr);
        btnClickbufr.loadFromFile("Data/Sounds/btnClick.wav");
        btnClick.setBuffer(btnClickbufr);

        //Create Cursor
        cursorTexture.loadFromFile("Data/Images/cursor.png");
        cursor.setTexture(cursorTexture);
        cursor.setScale(0.06f, 0.06f);

        //Create Background
        backgroundTexture.loadFromFile("Data/Images/Background1.jpg");
        background.setTexture(backgroundTexture);

        //Load Fonts
        BtnFont.loadFromFile("Data/Fonts/fontBtn.ttf");
    }

    void Logic()
    {
        //Moving Cursor with mouse position
        cursor.setPosition(mousePos);

        leftMouseBtn = 0; //Realese Left Click Button
    }

    //Render(Draw) Background
    void renderBG(sf::RenderWindow& window)
    {
        window.draw(background);
    }

    //Render(Draw) Cursor
    void renderCursor(sf::RenderWindow& window)
    {
        window.draw(cursor);
    }

}global;

/// Gravity Struct, responsible for applying gravity to different objects, giving it realistic feeling
struct Gravity
{
    bool inAir = 0, solid=0;
    float dv = 1.5f, maxVY = 100.0f, lostE = 0.35f, groundFr = 0.25f;

    void activate(sf::Sprite& body, sf::Vector2f& bodyV)
    {
        inAir = body.getPosition().y + body.getGlobalBounds().height / 2 < groundTop; //If object is above Air

        if(!solid)
        {
            if (inAir) //return it back to ground
            {
                if (bodyV.y < maxVY) bodyV.y += dv;
            }
            else      //bounce back with losing energy
            {
                bodyV.y = -bodyV.y + bodyV.y * lostE;
                bodyV.x -= bodyV.x * groundFr;
                body.setPosition(body.getPosition().x, groundTop - body.getGlobalBounds().height / 2 + 5);
            }
        }
    }
};

/// Object Struct, contains  Player, Ball, and any other moving objects (if exists)
struct Object
{
    struct Player
    {
        //VARIABLES

        bool up = 0, down = 0, right = 0, left = 0; //Movement Booleans
        const float lostE = 0.25f;

        //Character
        sf::Texture texture;
        sf::Sprite sprite;
        sf::Vector2f velocity;

        //Character Boundries
        float currentTopPos;
        float currentBottomPos;
        float currentLeftPos;
        float currentRightPos;

        //Shadow
        sf::RectangleShape shadow;

        //Animation
        bool LPlyr = 1;
        int width = 0;
        int height = 0;
        int imgCnt = 0;

        Gravity gravity; //chracter gravity
        //FUNCTIONS

        void create(std::string path, sf::Vector2f pos,bool LeftPlayer)
        {
            //Setting if it's left or right player
            LPlyr = LeftPlayer;

            //Body
            texture.loadFromFile(path);
            width = texture.getSize().x / 3;
            height = texture.getSize().y;
            sprite.setTexture(texture);
            if(LeftPlayer)
                sprite.setTextureRect(sf::IntRect(imgCnt * width, 0, width, height));
            else
                sprite.setTextureRect(sf::IntRect((imgCnt + 1) * width, 0, -1 * width, height));
            sprite.setOrigin(sprite.getGlobalBounds().width / 2, sprite.getGlobalBounds().height / 2);
            sprite.setPosition(pos);
            
             //Shadow
            shadow.setSize({sprite.getGlobalBounds().width, shadowThickness});
            shadow.setFillColor(sf::Color(0,0,0, 125));
            shadow.setOrigin(shadow.getSize().x / 2.0f, shadow.getSize().y / 2.0f);
            shadow.setPosition(sprite.getPosition().x,groundTop + shadowThickness / 2.0f);

            //Physics
            gravity.lostE = 0.9f;
            gravity.dv = 0.15f;
        }

        void move()
        {
            currentTopPos = sprite.getGlobalBounds().top;
            currentBottomPos = currentTopPos + sprite.getGlobalBounds().height;
            currentLeftPos = sprite.getGlobalBounds().left;
            currentRightPos = currentLeftPos + sprite.getGlobalBounds().width;

            //Controls
            if (up)
            {
                velocity.y = -6.0f;
                up = 0;
            }
            else gravity.activate(sprite, velocity);

            if (right && currentRightPos < screenWidth) velocity.x = 2.5f;
            if (left && currentLeftPos > 0) velocity.x = -2.5f;

            animateKick();

            //Screen Boundries
            if (sprite.getGlobalBounds().left <= 0) //Left Boundries
            {
                sprite.setPosition(sprite.getGlobalBounds().width / 2, sprite.getPosition().y);
            }

            if (sprite.getGlobalBounds().left + sprite.getGlobalBounds().width >= screenWidth) //Right Boundries
            {
                sprite.setPosition(screenWidth - sprite.getGlobalBounds().width / 2, sprite.getPosition().y);
            }

            //Movement Action
            sprite.move(velocity);

            //Moving shadow with the player
            shadow.setSize({sprite.getGlobalBounds().width * sprite.getPosition().y / screenHeight, shadowThickness}); // r = R * h/x , r = radius of shadow, R = maximum width of shadow, h total height = screenHeight, x = height of player
            shadow.setOrigin(shadow.getSize().x / 2.0f, shadow.getSize().y / 2.0f);
            shadow.setPosition(sprite.getPosition().x, groundTop + shadowThickness / 2.0f); //sync with player position in x and be under it in y

        }

        void moveAI(sf::Sprite& ball)
        {
            //Default Movement
            move();

            //Jump if ball in air and difference between altitude of player and ball bigger than 100 pixel
            if (abs(static_cast<int>(sprite.getPosition().y - ball.getPosition().y)) > 30)
                upPressed();
            else //Not in the air
            {
                upRealesed();
                
                //Kick Ball when near it
                if(LPlyr)
                {
                    //Kick only when the ball at right
                    if(sprite.getPosition().x - ball.getPosition().x >= -60)
                        downPressed();
                    else
                        downRealesed();
                }
                else
                {
                    //Kick only when the ball at left
                    if(sprite.getPosition().x - ball.getPosition().x <= 60)
                        downPressed();
                    else
                        downRealesed();  
                }
                
            }
           
            //Move Towards it
            if(ball.getPosition().x > sprite.getPosition().x)
            {
                rightPressed();
                leftRealesed();
            }
            else
            {
                rightRealesed();
                leftPressed();
            }
        }

        //Animtaion

        void animateKick()
        {
            if (down)
            {
                if (LPlyr == 1)
                {
                    if (imgCnt < 2)
                        imgCnt++;
                    sprite.setTextureRect(sf::IntRect(imgCnt * width, 0, width, height));
                }
                else if (LPlyr == 0)
                {
                    if (imgCnt < 3)
                        imgCnt++;

                    sprite.setTextureRect(sf::IntRect(imgCnt * width, 0, -1 * width, height));
                }
            }
            else
            {
                if (LPlyr == 1)
                {
                    imgCnt = 0;
                    sprite.setTextureRect(sf::IntRect(imgCnt * width, 0, width, height));
                }
                else if (LPlyr == 0)
                {
                    imgCnt = 1;
                    sprite.setTextureRect(sf::IntRect(imgCnt * width, 0, -1 * width, height));
                }
            }

        }

        //Collisions

        bool ballCollision(sf::Sprite& body, sf::Vector2f& bodyV, bool& solid)
        {
            bool atRight = body.getPosition().x > sprite.getPosition().x;
            bool atLeft = body.getPosition().x < sprite.getPosition().x;
            bool atAbove = body.getPosition().y < sprite.getPosition().y;
            bool atBottom = body.getPosition().y > sprite.getPosition().y;

            if (sprite.getGlobalBounds().intersects(body.getGlobalBounds())) //Collision Detection
            {
                if (atLeft && velocity.x < 0) //Ball is at Left of the Player
                {
                    bodyV.x = velocity.x * 3;
                }
                else if (atRight && velocity.x > 0) //Ball is at Right of the Player
                {
                    bodyV.x = velocity.x * 3;
                }
                else
                {
                    bodyV.x = -bodyV.x + bodyV.x * 0.8f;
                    return false;
                }

                if (atAbove) //Ball is above the Player
                {
                    if (velocity.y < 0)
                        bodyV.y = velocity.y * 3;
                    else if(!atBottom)
                    {
                        bodyV.y = -bodyV.y - 2.0f + bodyV.y * 0.7f;
                        bodyV.x > 0 ? bodyV.x += 1.5f : bodyV.x -= 1.5f ;
                    }
                }               
                else if (atBottom) //Ball is at bottom of the Player
                {
                    if (velocity.y > 0)
                    {
                        bodyV.y = velocity.y * 3;
                        
                        if (velocity.x == 0)
                            bodyV.x += 2.5;
                    }
                    else bodyV.y = -bodyV.y + bodyV.y * 0.8f;
                }

                if (down && !(body.getPosition().y <= sprite.getPosition().y - 5)) //Kicking ball if down pressed
                {
                    if (LPlyr && body.getPosition().x >= sprite.getPosition().x)
                    {
                        bodyV = { 20, -27 };
                    }
                    else if (!LPlyr && body.getPosition().x <= sprite.getPosition().x)
                        bodyV = { -20, -27 };
                }

                return true;
            }

            solid=0;
            gravity.solid=0;
            return false;
        }
        
        void playerColl(sf::Sprite& body)
        {
            bool atRight = body.getPosition().x > sprite.getPosition().x;
            bool atLeft = body.getPosition().x < sprite.getPosition().x;
            bool atBottom = body.getPosition().y > sprite.getPosition().y;

            if(sprite.getGlobalBounds().intersects(body.getGlobalBounds()))
            {
                if(atBottom)
                {
                    gravity.solid=1;
                    velocity.y=0;
                }
                
                if (atLeft)
                {
                    leftRealesed();
                }
                else if(atRight)
                {
                    rightRealesed();
                }
                velocity.x=0;
            }
            else gravity.solid=0;
        }

        
        //Pressed button
        void upPressed()
        {
            if (sprite.getPosition().y + sprite.getGlobalBounds().height / 2 >= groundTop) //Jump
                up = 1;
        }

        void downPressed()
        {
            down = 1;
        }

        void rightPressed()
        {
            right = 1;
        }

        void leftPressed()
        {
            left = 1;
        }

        //Released button

        void upRealesed()
        {
            up = 0;
        }

        void downRealesed()
        {
            down = 0;
        }

        void rightRealesed()
        {
            right = 0;
        }

        void leftRealesed()
        {
            left = 0;
        }

        ///Rendering
        void render(sf::RenderWindow &window)
        {
            window.draw(sprite);
            window.draw(shadow);
        }
    };

    struct Ball
    {
        ////VARIABLES

        // Ball Propreties
        sf::Texture texture;
        sf::Sprite sprite;

        //Shadow
        sf::RectangleShape shadow;

        const float radius = 25;
        const float lostE = 0.25f;
        // Physics
        Gravity gravity;
        sf::Vector2f velocity;

        void create()
        {
            //Body
            texture.loadFromFile("Data/Images/ball.png");
            sprite.setTexture(texture);
            sprite.setOrigin(sf::Vector2f(25, 25));
            sprite.setPosition(sf::Vector2f(500, 100));
            sprite.setScale(0.80f, 0.80f);
            
            //Shadow
            shadow.setSize({25, shadowThickness});
            shadow.setFillColor(sf::Color(0,0,0, 125));
            shadow.setOrigin(shadow.getSize().x / 2.0f, shadow.getSize().y / 2.0f);
            shadow.setPosition(sprite.getPosition().x,groundTop + 5 / 2.0f);

            //Physics Properties
            gravity.dv = 0.5;
            gravity.groundFr = 0.0f;
            // randV();
        }

        void randV()
        {
            srand(time(NULL));

            velocity = { rand() % 11 - 5.0f, 0};
        }
        
        void move()
        {
            //Screen Boundries
            if (sprite.getGlobalBounds().left <= 0) //Left Boundries
            {
                sprite.setPosition(sprite.getGlobalBounds().width / 2, sprite.getPosition().y);
                velocity.x = -velocity.x + velocity.x * lostE;
            }
            if (sprite.getGlobalBounds().left + sprite.getGlobalBounds().width >= screenWidth) //Right Boundries
            {
                sprite.setPosition(screenWidth - sprite.getGlobalBounds().width / 2, sprite.getPosition().y);
                velocity.x = -velocity.x + velocity.x * lostE;
            }

            //Activating Gravity
            gravity.activate(sprite, velocity);

            //Moving Ball
            sprite.move(velocity);
            sprite.rotate(velocity.x);

            //Moving shadow with the ball
            shadow.setSize({30 * sprite.getPosition().y / screenHeight, shadowThickness}); // r = R * h/x , r = radius of shadow, R = maximum radius of shadow, h total height = screenHeight, x = height of ball
            shadow.setOrigin(shadow.getSize().x / 2.0f, shadow.getSize().y / 2.0f);
            shadow.setPosition(sprite.getPosition().x, groundTop + 5/2.0f); //sync with ball position in x and be under it in y
        }

        void goalCollision(sf::Sprite& body)
        {
            sf::FloatRect goalTop = { body.getGlobalBounds().left, body.getGlobalBounds().top, body.getGlobalBounds().width, 40 };
            if (goalTop.intersects(sprite.getGlobalBounds()))
            {
                velocity.y = -velocity.y + velocity.y * lostE;
                gravity.solid=1;
            }
            else
            {
                gravity.solid=0;
            }
            
        }

        void render(sf::RenderWindow &window)
        {
            window.draw(sprite);
            window.draw(shadow);
        }
    };
};

/// Button Struct, responsible for making and rendering all different buttons in the game
struct Button
{
    /// Responsible for Rectangular Shaped Buttons (Added by Tawfik)
    struct Rectangular
    {
        sf::Texture texture;
        sf::Vector2f size;
        sf::Sprite sprite;
        sf::Text title;

        bool inside = 0;
        bool disabled=0;

        void create(sf::Vector2f pos, std::string x)
        {
            // Button style and dimensions
            texture.loadFromFile("Data/Images/RecButton.png");
            size = sf::Vector2f(static_cast<float>(texture.getSize().x), static_cast<float>(texture.getSize().y));
            size.x /= 3;
            sprite.setTexture(texture);
            notHoveredTexture();
            sprite.setScale(1, 0.5);
            sprite.setOrigin(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));
            sprite.setPosition(pos);

            // Text inside button
            title.setFont(global.BtnFont);
            title.setCharacterSize(50);
            title.setString(x);
            title.setOrigin(title.getLocalBounds().width / 2, title.getGlobalBounds().height / 2);
            title.setPosition(sprite.getPosition().x, sprite.getPosition().y - 15);
        }

        /// Next 3 functions change the texture based on its state

        void notHoveredTexture()
        {
            sprite.setTextureRect(sf::IntRect(0, 0, static_cast<int>(size.x), static_cast<int>(size.y)));
        }

        void hoveredTexture()
        {
            sprite.setTextureRect(sf::IntRect(static_cast<int>(size.x), 0, static_cast<int>(size.x), static_cast<int>(size.y)));
        }

        void disabledTexture()
        {
            sprite.setTextureRect(sf::IntRect(static_cast<int>(size.x) * 2, 0, static_cast<int>(size.x), static_cast<int>(size.y)));
            disabled=1;
        }

        //Checks if Mouse Hovered button
        bool mouseHover()
        {
            if(!disabled)
            {
                if (sprite.getGlobalBounds().contains(global.mousePos))
                {
                    if (!inside)
                    {
                        hoveredTexture();
                        if (global.soundEnabled)
                            global.btnHover.play();
                        inside = 1;
                    }
                    return true;
                }
                else
                {
                    if (inside)
                        notHoveredTexture();
                    inside = 0;
                    return false;
                }
            }
            else return false;
        }

        //Checks if Mouse Pressed
        bool mouseLeftClicked()
        {
            if (mouseHover() && global.leftMouseBtn)
            {
                if (global.soundEnabled)
                    global.btnClick.play();
                global.leftMouseBtn = 0;
                return true;
            }
            else return false;
        }

        void render(sf::RenderWindow& window)   // draws the button and its text
        {
            window.draw(sprite);
            window.draw(title);
        }
    };

    /// Responsible for Round Shaped Buttons (Added by Menna)
    struct Round
    {
        sf::Texture Tex;
        sf::Vector2f size;
        sf::Sprite sprite;
        bool inside = 0;

        void disabledTexture()
        {
            sprite.setTextureRect(sf::IntRect(static_cast<int>(size.x), 0, static_cast<int>(size.x), static_cast<int>(size.y)));
        }

        void notHoveredTexture()
        {
            sprite.setTextureRect(sf::IntRect(0, 0, static_cast<int>(size.x), static_cast<int>(size.y)));
        }

        void hoveredTexture()
        {
            sprite.setTextureRect(sf::IntRect(static_cast<int>(size.x), 0, static_cast<int>(size.x), static_cast<int>(size.y)));
        }

        void create(std::string Type)
        {
            Tex.loadFromFile("Data/Images/" + Type + ".png");

            sprite.setTexture(Tex);

            if (Type == "Cancel")
                sprite.setScale(0.15f, 0.15f);
            else
                sprite.setScale(0.4f, 0.4f);

            sprite.setOrigin(static_cast<float>(Tex.getSize().x) / 6, static_cast<float>(Tex.getSize().y) / 6);

            size = { static_cast<float>(Tex.getSize().x), static_cast<float>(Tex.getSize().y) };
            size.x /= 3;
            notHoveredTexture();
        }

        bool mouseHover()
        {
            if (sprite.getGlobalBounds().contains(global.mousePos))
            {
                if (!inside)
                {
                    hoveredTexture();
                    if (global.soundEnabled)
                        global.btnHover.play();
                    inside = 1;
                }
                return true;
            }
            else
            {
                if (inside)
                    notHoveredTexture();
                inside = 0;
                return false;
            }
        }

        bool mouseLeftClicked()
        {
            if (mouseHover() && global.leftMouseBtn)
            {
                if (global.soundEnabled)
                    global.btnClick.play();
                global.leftMouseBtn = 0;
                return true;
            }
            return false;
        }

    };

};

//creating levels, (Added by Mariam) 
struct Levels
{
    std::string player[4] = { "Data/Images/BossBabySheet.png",  "Data/Images/ButterSheet.png", "Data/Images/GumballSheet.png", "Data/Images/MegaSheet.png" };
    std::string enemy[4] = { "Data/Images/TimSheet.png", "Data/Images/MojoSheet.png", "Data/Images/RobSheet.png", "Data/Images/MetroSheet.png" };
    int crntLvl = 0;
    //Level text
    sf::Font font;
    sf::Text text;

    //Functions

    void create()
    {
        font.loadFromFile("Data/Fonts/fontBtn.ttf");
        text.setFont(font);
        text.setString("Level " + std::to_string(crntLvl + 1));
        text.setCharacterSize(50);
        text.setOrigin(text.getGlobalBounds().width / 2, text.getGlobalBounds().height / 2);
        text.setPosition(screenWidth / 2, 50);
        text.setFillColor(sf::Color::Black);
        text.setStyle(sf::Text::Bold);
    }

    void setText()
    {
        text.setString("Level " + std::to_string(crntLvl + 1));
    }

    void nextlevel(char& screen, bool& LevelEnded)
    {

        if (++crntLvl < 4)
        {
            LevelEnded = 0;
        }
        else
        {
            crntLvl = 0;
            screen = 'c';
        }

        setText();
    }


    void render(sf::RenderWindow& window)
    {
        window.draw(text);
    }
};

/// Match Struct, contains the main game screen
struct Match
{
    struct Mode
    {
        //// VARIABLES //// 
        
        Object::Player player1, player2; //Players
    
        Object::Ball ball; //Ball

        //Sounds
        bool isGameEndSound=0;
        
        //Scores
        int Score1 = 0, Score2 = 0;
        bool inside_goal = 0, outside_goal = 0;
        bool EndGame=0;
        
        //Timer
        int timer;
        const int timerFull= 60*60;
        
        //// FUNCTIONS ////
        void create(Match& parent)
        {
            player1.create((parent.levels.player[parent.levels.crntLvl]), {120, 550}, 1);
            player2.create((parent.levels.enemy[parent.levels.crntLvl]), {880, 550}, 0);
            ball.create();
            timer = timerFull;
        }
        
        void collisions(Match& parent)
        {
            if ( (player1.ballCollision(ball.sprite, ball.velocity, ball.gravity.solid) || player2.ballCollision(ball.sprite, ball.velocity, ball.gravity.solid)) && global.soundEnabled)// || player2.stopCollision(ball.sprite,ball.velocity))
                parent.kickBallSound.play();

            ball.goalCollision(parent.goal1);
            ball.goalCollision(parent.goal2);
            
            player1.playerColl(player2.sprite);
            player2.playerColl(player1.sprite);
        }

        void countScore(Match& parent)
        {
            if (!(ball.sprite.getGlobalBounds().intersects(parent.innerGoal1) ||
            ball.sprite.getGlobalBounds().intersects(parent.innerGoal2)))
                outside_goal = 1;
            if (!inside_goal && outside_goal && ball.sprite.getGlobalBounds().intersects(parent.innerGoal1) )
            {
                Score2++;
                goalScored(parent);
            }
            else if (!inside_goal && outside_goal && ball.sprite.getGlobalBounds().intersects(parent.innerGoal2))
            {
                Score1++;
                goalScored(parent);
            }
            else
                inside_goal = 0;
        }
        
        void goalScored(Match& parent)
        {
            inside_goal = 1;
            outside_goal = 0;
            if(global.soundEnabled)
                parent.GoalScoredSnd.play();
            restart();
        }

        void restart()
        {
            ball.sprite.setPosition(500, 100);
            ball.randV();

            player1.sprite.setPosition(120.0f, 550.0f);
            player1.velocity = {};
            
            player2.sprite.setPosition(880.0f, 550.0f);
            player2.velocity = {};
        }
        
        void newLvl(Match& parent)
        {
            player1.create((parent.levels.player[parent.levels.crntLvl]), {120, 550}, 1);
            player2.create((parent.levels.enemy[parent.levels.crntLvl]), {880, 550} , 0);
            
            if(global.soundEnabled)
            parent.GameStartSound.play();
            
            restart();

            Score1 = Score2 = isGameEndSound = parent.startedClock = EndGame = 0;
            timer = timerFull;
        }
    };
    
    //// VARIABLES ////

    // Creating Modes
    Mode single,multi;
    
    // Textures declaration
    sf::Texture gl;

    // Bodies declaration

    //Goals
    sf::Sprite goal1, goal2; //Sprites
    sf::FloatRect innerGoal1,innerGoal2; //Inner of Goal

    // Sounds
    sf::SoundBuffer GameStartBuff;
    sf::Sound GameStartSound;
 
    sf::SoundBuffer kickBallBuff;
    sf::Sound kickBallSound;

    sf::SoundBuffer GoalScoredBuff;
    sf::Sound GoalScoredSnd;

    sf::SoundBuffer cntDwnBuff;
    sf::Sound cntDwnSound;
    
    sf::SoundBuffer GameEndBuff;
    sf::Sound GameEndSound;
    
    // Buttons
    Button::Round pauseBtn;

    // Scores
    sf::Text score1, score2;
    sf::Text winOrlose;

    // Timer
    sf::Text timer_cnt;
    sf::Clock c1;
    bool startedClock=0;

    // Levels
    Levels levels;

    //Instructions Menu Cancel Button Action
    char crntMode;  //h for home, s for single player, m for multiplayer

    //// FUNCTIONS ////

    void create()
    {
        //Modes
        single.create(*this);
        multi.create(*this);

        //Goals
        gl.loadFromFile("Data/Images/Goal.png");
        goal1.setTexture(gl);
        goal1.setOrigin(sf::Vector2f(50, 90));
        goal1.setPosition(sf::Vector2f(20, 500));
        innerGoal1 = sf::FloatRect(goal1.getGlobalBounds().left, goal1.getGlobalBounds().top + 40, goal1.getGlobalBounds().width - 40, goal1.getGlobalBounds().height - 40);
     
        goal2.setTexture(gl);
        goal2.setTextureRect(sf::IntRect(gl.getSize().x, 0, -1 * gl.getSize().x, gl.getSize().y));
        goal2.setOrigin(sf::Vector2f(50, 90));
        goal2.setPosition(sf::Vector2f(980, 500));
        innerGoal2 = sf::FloatRect(goal2.getGlobalBounds().left, goal2.getGlobalBounds().top + 40, goal2.getGlobalBounds().width - 40, goal2.getGlobalBounds().height - 40);
     
        //Sounds
        GameStartBuff.loadFromFile("Data/Sounds/gStart.wav");
        GameStartSound.setBuffer(GameStartBuff);

        kickBallBuff.loadFromFile("Data/Sounds/Kick.wav");
        kickBallSound.setBuffer(kickBallBuff);

        GoalScoredBuff.loadFromFile("Data/Sounds/goalScored.wav");
        GoalScoredSnd.setBuffer(GoalScoredBuff);

        cntDwnBuff.loadFromFile("Data/Sounds/cntDwn.wav");
        cntDwnSound.setBuffer(cntDwnBuff);

        GameEndBuff.loadFromFile("Data/Sounds/gEnd.wav");
        GameEndSound.setBuffer(GameEndBuff);
        GameEndSound.setVolume(20.0f);

        //Pause Button
        pauseBtn.create("PauseButton");
        pauseBtn.sprite.setPosition(screenWidth - 80, 30);
        pauseBtn.sprite.setScale(0.25, 0.25);

        // Scores
        score1.setFont(global.BtnFont);
        score2.setFont(global.BtnFont);
        score1.setCharacterSize(50);
        score2.setCharacterSize(50);
        score1.setPosition(screenWidth / 3, 60);
        score2.setPosition(2 * screenWidth / 3, 60);

        // Timer
        timer_cnt.setFont(global.BtnFont);
        timer_cnt.setCharacterSize(40);
        timer_cnt.setPosition(screenWidth / 15, 30);
        
        //Match Result
        winOrlose.setFont(global.BtnFont);
        winOrlose.setString("You Lose");
        winOrlose.setStyle(sf::Text::Style::Bold);
        winOrlose.setCharacterSize(80);
        winOrlose.setOrigin(winOrlose.getGlobalBounds().width / 2.0f, winOrlose.getGlobalBounds().height / 2.0f);
        winOrlose.setPosition(screenWidth / 2, screenHeight / 2);
        winOrlose.setFillColor(sf::Color::Green);

        //level text
        levels.create();
    }

    // Logic

    ///SinglePlayer
    void SingleLogic(char& screen)
    {
        single.collisions(*this);

        // Movement Control
        single.player1.move();
        single.player2.moveAI(single.ball.sprite);
        single.ball.move();

        scoringSingle(screen);
    }

    void scoringSingle(char& screen)
    {
        // Scoring
        single.countScore(*this);

        // End of Game
        if (single.timer <= 0)
        {
            if (single.Score1 != single.Score2)
            {
                winOrlose.setString(single.Score1 > single.Score2 ? "You Won" : "You Lost");

                if(showResults())
                {
                    if(single.Score1 < single.Score2) //If Lost
                    {
                        screen = 'h'; //Go home
                    }
                    else               //if Won
                    {
                        levels.nextlevel(screen, single.EndGame);  //Goto Next Level
                    }
                    single.newLvl(*this);
                }
                else
                {
                    if(!single.isGameEndSound)
                    {
                        if(global.soundEnabled)
                            GameEndSound.play();
                        single.isGameEndSound=1;
                    }

                    single.EndGame=1;
                }

            }

        }
        else
        {
            if(single.timer == 60 * 5 && global.soundEnabled)
                cntDwnSound.play();

            single.timer--;
        }

        score1.setString(std::to_string(single.Score1));
        score2.setString(std::to_string(single.Score2));
        timer_cnt.setString(std::to_string(single.timer / 60) + ":0" + std::to_string(single.timer >= 0 ? (single.timer / 6) % 10 : 9 - (-multi.timer / 6) % 10));

    }

    void newGame()
    {
        levels.crntLvl=0;
        single.newLvl(*this);
        levels.setText();
    }

    //MultiPlayer
    void MultiLogic(char& screen)
    {
        multi.collisions(*this);

        // Movement Control
        multi.player1.move();
        multi.player2.move();
        multi.ball.move();

        scoringMulti(screen);
    }

    void scoringMulti(char& screen)
    {
        // Scoring
        multi.countScore(*this);

        // End of Game
        if (multi.timer <= 0)
        {
            if (multi.Score1 != multi.Score2)
            {
                winOrlose.setString(multi.Score1 > multi.Score2 ? "Left Player Won" : "Right Player Won");

                if(showResults())
                {
                    screen='h';
                    multi.newLvl(*this);
                }
                else
                {
                    if(!multi.isGameEndSound)
                    {
                        if(global.soundEnabled)
                            GameEndSound.play();
                        multi.isGameEndSound=1;
                    }
                    multi.EndGame=1;
                }

            }

        }
        else
        {
            if(multi.timer == 60 * 5 && global.soundEnabled)
                cntDwnSound.play();

            multi.timer--;
        }

        score1.setString(std::to_string(multi.Score1));
        score2.setString(std::to_string(multi.Score2));
        timer_cnt.setString(std::to_string(multi.timer / 60) + ":0" + std::to_string(multi.timer >= 0 ? (multi.timer / 6) % 10 : 9 - (-multi.timer / 6) % 10));
    }

    //Common Logic
    bool showResults()
    {
        if(!startedClock)
        {
            startedClock=1;
            c1.restart();
        }
        else if(c1.getElapsedTime().asSeconds() >= 5)
        {
            return true;
        }
        return false;
    }

    void PauseLogic(char& session)
    {
        if (pauseBtn.mouseLeftClicked())
            global.GamePaused = 1;
    }
    
    // Rendering

    //SinglePlayer
    void renderSingle(sf::RenderWindow& window)
    {
        if(!single.EndGame)
        {
            levels.render(window);
            single.ball.render(window);
            single.player1.render(window);
            single.player2.render(window);
            window.draw(goal1);
            window.draw(goal2);
            window.draw(pauseBtn.sprite);
            window.draw(score1);
            window.draw(score2);
            window.draw(timer_cnt);
        }
        else
        {
            window.draw(winOrlose);
        }

    }

    //MultiPlayer
    void renderMulti(sf::RenderWindow& window)
    {
        if(!multi.EndGame)
        {
            multi.ball.render(window);
            multi.player1.render(window);
            multi.player2.render(window);
            window.draw(goal1);
            window.draw(goal2);
            window.draw(pauseBtn.sprite);
            window.draw(score1);
            window.draw(score2);
            window.draw(timer_cnt);
        }
        else
        {
            window.draw(winOrlose);
        }

    }

};

/// Menu Struct, contains all different menus of the game
struct Menu
{
    /// Main Menu, (Added by Tawfik)
    struct Main
    {
        /////////////////VARIABLES
        
        //Logo
        sf::Texture logoTex;
        sf::Sprite logo;

        //Music
        sf::Music BGMusic;
        bool MusicPlaying = 0;
        
        //Balls
        Object::Ball ball[2];

        //Buttons
        static const int noOfBtns = 5;
        Button::Rectangular btn[noOfBtns];
        std::string btnTitle[noOfBtns] = { "New Game", "Continue", "MultiPlayer", "Instructions", "Credits" };
        char btnSession[noOfBtns] = { 's', 's', 'm', 'i', 'c' };

        /////////////////FUNCTIONS

        //Creating Objects
        void create()
        {
            //Balls
            for (int i = 0; i < 2; i++)
            {
                //Loading Texture and Setting Postion
                ball[i].create();
                ball[i].sprite.setPosition(screenWidth / 1.5f * (i+0.25f), screenHeight / 4 * 1.5f);
                
                //Make Them bouncy balls
                ball[i].velocity = {};
                ball[i].gravity.lostE = 0;
                ball[i].gravity.groundFr = 0;
            }
            

            //Create Buttons : Divided spaces in screen into 8 Xs and 7 Ys to put buttons in order
            for (char i = 0; i < noOfBtns; i++)
            {
                btn[i].create(sf::Vector2f(screenWidth / 2, screenHeight / 6.5f * (i + 2)), btnTitle[i]);
            }

            //Disable Continue Button
            btn[1].disabledTexture();

            //Load and Play Music
            BGMusic.openFromFile("Data/Sounds/MainMenu.wav");
            BGMusic.setLoop(true);

            //Logo
            logoTex.loadFromFile("Data/Images/logo.png");
            logoTex.setSmooth(1);
            logo.setTexture(logoTex);
            logo.setScale(1.5f,1.7f);
            logo.setPosition(182, 13);
        }

        //Logic

        //When mouse hovers over buttons
        void Logic(char& session,Match& Game)
        {
            //Play Music
            if (!MusicPlaying && global.soundEnabled)
            {
                BGMusic.play();
                MusicPlaying = 1;
            }

            //Buttons Hovered or Clicked Actions
            for (int i = 0; i < noOfBtns; i++)
            {
                if (btn[i].mouseLeftClicked())
                {
                    session = btnSession[i];
                    BGMusic.stop();
                    MusicPlaying = 0;

                    switch (i)
                    {
                    case 0:
                        Game.newGame();
                        btn[1].disabled=0;
                        btn[1].notHoveredTexture();
                        break;
                    }
                }
            }

            //Move Balls
            for (int i = 0; i < 2; i++)
            {
                ball[i].move();
            }
            
        }

        //Rendering
        void render(sf::RenderWindow& window)
        {
            window.draw(logo);

            for (int i = 0; i < noOfBtns; i++)
            {
                btn[i].render(window);
            }

            for (int i = 0; i < 2; i++)
            {
                ball[i].render(window);
            }
        }
    };

    /// Credits Menu, (Added by Mariam)
    struct Credits
    {

        //credits photo
        sf::Texture creditstexture;
        sf::Sprite credits;

        //credits Background Music
        sf::Music BGMusic;
        bool isPlaying = 0;

        //Return Button
        Button::Round returnBtn;

        //functions
        //creating credits
        void create()
        {
            //Credits Image
            creditstexture.loadFromFile("Data/Images/credits.jpg");
            credits.setTexture(creditstexture);

            //Background Music
            BGMusic.openFromFile("Data/Sounds/Credits.wav");
            BGMusic.setLoop(1);

            //Cancel Button
            returnBtn.create("Cancel");
            returnBtn.sprite.setPosition(screenWidth - 50, 30);
            returnBtn.sprite.setScale(0.25f, 0.25f);
        }

        void Logic(char& session)
        {
            if (!isPlaying && global.soundEnabled)
            {
                BGMusic.play();
                isPlaying = 1;
            }

            if (returnBtn.mouseLeftClicked())
            {
                BGMusic.stop();
                isPlaying = 0;
                session = 'h';
            }
        }

        //Rendering
        void render(sf::RenderWindow& window)
        {
            window.draw(credits);
            window.draw(returnBtn.sprite);
        }

    };

    /// Instructions Menu, (Added by Mariam)
    struct Instructions
    {
        //instructions photo
        sf::Texture instructionstexture;
        sf::Sprite instructions;

        //Cancel Button
        Button::Round returnBtn;

        //functions
        //creating credits
        void create()
        {
            instructionstexture.loadFromFile("Data/Images/instructions.jpg");
            instructions.setTexture(instructionstexture);
            returnBtn.create("Cancel");
            returnBtn.sprite.setPosition(screenWidth / 9 * 7, 10);
            returnBtn.sprite.setScale(0.25f, 0.25f);
        }

        //Button Logic
        void Logic(char& session,char& crntMode)
        {
            if (returnBtn.mouseLeftClicked())
            {
                if (crntMode == 's')
                {
                    global.GamePaused = 1;
                    session = 's';
                }
                else if(crntMode == 'm')
                {
                    global.GamePaused = 1;
                    session = 'm';
                }
                else
                {
                    global.GamePaused=0;
                    session = 'h';
                }
                
            }
        }

        //Rendering
        void render(sf::RenderWindow& window)
        {
            window.draw(instructions);
            window.draw(returnBtn.sprite);
        }

    };

    /// Pause Menu, (Added by Menna)
    struct Pause
    {
        // VARIABLES
        static const int n = 5;
        Button::Round btn[n];

        //Background for Buttons
        sf::Texture bgT;
        sf::Sprite bg;

        //Semi Transparent Background
        sf::RectangleShape Blur;

        //Title
        sf::Text paused;

        //Mute Button Texture
        sf::Texture unmuteTex;

        // FUNCTIONS
        void create()
        {
            // Background
            bgT.loadFromFile("Data/Images/pause menu.png");
            bg.setTexture(bgT);
            bg.setOrigin(bgT.getSize().x / 2.0f, bgT.getSize().y / 2.0f);
            bg.setScale(1.0f, 1.05f);
            bg.setPosition(screenWidth / 2, screenHeight / 2 + 50);

            //Semi Transparent Background
            Blur.setSize(sf::Vector2f(screenWidth, screenHeight));
            Blur.setFillColor(sf::Color(220, 220, 220, 80));

            // Buttons
            
            // 1-resume 2-restart 3-mute 5-instructions 6-main menu
            sf::String s[] = { "Cancel","Restart","Mute", "Inst","Home" };
            for (int i = 0; i < n; i++)
            {
                btn[i].create(s[i]);
            }

            btn[0].sprite.setPosition(bgT.getSize().x + bgT.getSize().x / 4.0f - 20.0f, bgT.getSize().y - bgT.getSize().y / 4.0f + 20.0f); //Cancel Button
            btn[1].sprite.setPosition(screenWidth / 2 - 160 + 70, screenHeight / 2 - 70); //Restart Button
            btn[2].sprite.setPosition(screenWidth / 2 + 160 - 70, screenHeight / 2 - 70); //Mute Button
            unmuteTex.loadFromFile("Data/Images/Unmute.png");
            btn[3].sprite.setPosition(screenWidth / 2 - 160 + 70, screenHeight / 2 + 70); //Instructions Button
            btn[4].sprite.setPosition(screenWidth / 2 + 160 - 70, screenHeight / 2 + 70); //Home Button

            //Text

            //Pause Menu Title
            paused.setFont(global.BtnFont);
            paused.setCharacterSize(100);
            paused.setString("PAUSED");
            paused.setOrigin(paused.getLocalBounds().width / 2, paused.getGlobalBounds().height / 2);
            paused.setPosition(screenWidth / 2, screenHeight / 2 - 180);
        }

        void Logic(char& session,Match& Game)
        {
            for (int i = 0; i < n; i++)
            {
                if (btn[i].mouseLeftClicked())
                {
                    switch (i)
                    {
                    case 1: //Restart
                        if(session == 's')
                            Game.single.newLvl(Game);
                        else if(session == 'm')
                            Game.single.newLvl(Game);
                    case 0: //Cancel
                        global.GamePaused = 0;
                        break;
                    case 2: //Mute
                        if(global.soundEnabled)
                        {
                            global.soundEnabled = 0;
                            btn[2].sprite.setTexture(unmuteTex);
                        }
                        else
                        {
                            global.soundEnabled = 1;
                            btn[2].sprite.setTexture(btn[2].Tex);
                        }
                        break;
                    case 3: //Instructions
                        global.GamePaused = 0;
                        Game.crntMode = session;
                        session = 'i';
                        break;
                    case 4: //Home
                        global.GamePaused = 0;
                        Game.crntMode='h';
                        session = 'h';
                        break;
                    }
                }
            }
        }

        void render(sf::RenderWindow& window)
        {
            //Draw Background, Banner, Title
            window.draw(Blur);
            window.draw(bg);
            window.draw(paused);

            //Draw Buttons
            for (int i = 0; i < n; i++)
            {
                window.draw(btn[i].sprite);
            }
        }
    };
};

void loadScreen(sf::RenderWindow& window)
{
    sf::Text title;
    sf::Font titlefnt;
    titlefnt.loadFromFile("Data/Fonts/fontBtn.ttf");
    title.setFont(titlefnt);
    title.setString("Loading");
    title.setCharacterSize(100);
    title.setOrigin(title.getGlobalBounds().width / 2, title.getGlobalBounds().height / 2);
    title.setPosition(screenWidth / 2, screenHeight / 2);
    window.clear(sf::Color::Black);
    window.draw(title);
    window.display();
}

void saveData(bool ContinueBtn, Match& Game)
{
    std::ofstream file;

    file.open("Data/save.txt", std::ios_base::trunc);

    if(!file.good())
    {
        file.close();
        std::cout << "Error Saving Data\n";
        return;
    }

    //State Of Sound
    file << global.soundEnabled << '\n';

    //State Of Continue Btn in Main Menu
    file << ContinueBtn << '\n';

    //Current Achieved Level
    file << Game.levels.crntLvl << '\n';

    //Coordinates of players, ball and their scores in SinglePlayer
    file << Game.single.player1.sprite.getPosition().x << ' ' << Game.single.player1.sprite.getPosition().y << '\n';
    file << Game.single.player1.velocity.x << ' ' << Game.single.player1.velocity.y << '\n';
    
    file << Game.single.player2.sprite.getPosition().x << ' ' << Game.single.player2.sprite.getPosition().y << '\n';
    file << Game.single.player2.velocity.x << ' ' << Game.single.player2.velocity.y << '\n';
    
    file << Game.single.ball.sprite.getPosition().x << ' ' << Game.single.ball.sprite.getPosition().y << '\n';
    file << Game.single.ball.velocity.x << ' ' << Game.single.ball.velocity.y << '\n';
    
    file << Game.single.Score1 << ' ' << Game.single.Score2 << '\n';
    file << Game.single.timer << '\n';

    //Coordinates of players, ball and their scores in MultiPlayer
    file << Game.multi.player1.sprite.getPosition().x << ' ' << Game.multi.player1.sprite.getPosition().y << '\n';
    file << Game.multi.player1.velocity.x << ' ' << Game.multi.player1.velocity.y << '\n';
    
    file << Game.multi.player2.sprite.getPosition().x << ' ' << Game.multi.player2.sprite.getPosition().y << '\n';
    file << Game.multi.player2.velocity.x << ' ' << Game.multi.player2.velocity.y << '\n';
    
    file << Game.multi.ball.sprite.getPosition().x << ' ' << Game.multi.ball.sprite.getPosition().y << '\n';
    file << Game.multi.ball.velocity.x << ' ' << Game.multi.ball.velocity.y << '\n';
    
    file << Game.multi.Score1 << ' ' << Game.multi.Score2 << '\n';
    file << Game.multi.timer;

    file.close();
}

void loadData(sf::Sprite& muteBtn, sf::Texture& unmuteTex, bool& disabledContinue, Button::Rectangular& Continue, Match& Game)
{
    std::ifstream file;

    file.open("Data/save.txt");

    if(!file.good())
    {
        file.close();
        return;
    }
    
    //Restore Data

    //State Of Sounds
    file >> global.soundEnabled;
    if(!global.soundEnabled)
        muteBtn.setTexture(unmuteTex);

    //State of Continue Button in Main Menu
    file >> disabledContinue;
    if(!disabledContinue)
        Continue.notHoveredTexture();

    //last Achieved Level
    file >> Game.levels.crntLvl;
    Game.levels.setText();
    Game.single.create(Game);

    sf::Vector2f temp; //Temporary Variables to store positions
    
    //SinglePlayer
    file >> temp.x >> temp.y;
    Game.single.player1.sprite.setPosition(temp);
    file >> Game.single.player1.velocity.x >> Game.single.player1.velocity.y;

    file >> temp.x >> temp.y;
    Game.single.player2.sprite.setPosition(temp);
    file >> Game.single.player2.velocity.x >> Game.single.player2.velocity.y;

    file >> temp.x >> temp.y;
    Game.single.ball.sprite.setPosition(temp);
    file >> Game.single.ball.velocity.x >> Game.single.ball.velocity.y;

    file >> Game.single.Score1 >> Game.single.Score2;
    file >> Game.single.timer;

    //MultiPlayer
    file >> temp.x >> temp.y;
    Game.multi.player1.sprite.setPosition(temp);
    file >> Game.multi.player1.velocity.x >> Game.multi.player1.velocity.y;

    file >> temp.x >> temp.y;
    Game.multi.player2.sprite.setPosition(temp);
    file >> Game.multi.player2.velocity.x >> Game.multi.player2.velocity.y;

    file >> temp.x >> temp.y;
    Game.multi.ball.sprite.setPosition(temp);
    file >> Game.multi.ball.velocity.x >> Game.multi.ball.velocity.y;

    file >> Game.multi.Score1 >> Game.multi.Score2;
    file >> Game.multi.timer;

    file.close();
}

int main()
{
    //Creating window
    sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Head Soccer", sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(60);
    window.setMouseCursorVisible(false);

    //Loading Screen till Resources Loads
    loadScreen(window);

    //variables
    char screen = 'h'; // to know which screen to render and handle

    /*
        h(Home) Main Menu
        s(Single) SinglePlayer
        l(Level Selection)
        c(Credits)
        m(Multiplayer)
        i(instructions)
    */

    //Global Objects
    global.create();

    //Main Menu
    Menu::Main main;
    main.create();

    //Credits//mariam//
    Menu::Credits credits;
    credits.create();

    //Instructions//mariam//
    Menu::Instructions instructions;
    instructions.create();

    //Pause Menu
    Menu::Pause pause;
    pause.create();

    //Single Player Session
    Match Game;
    Game.create();

    //Load Data Saved Form File
    loadData(pause.btn[2].sprite, pause.unmuteTex, main.btn[1].disabled, main.btn[1], Game);
    
    //Game Loop
    while (window.isOpen())
    {
        //Events
        sf::Event e;
        while (window.pollEvent(e))
        {
            switch (e.type)
            {
                //Window Events
            case sf::Event::Closed:
                saveData(main.btn[1].disabled, Game);
                window.close();
                break;
            case sf::Event::LostFocus:
                if (screen == 's')
                    global.GamePaused = 1;
                break;

                //Mouse Events
            case sf::Event::MouseMoved:
                global.mousePos = sf::Vector2f(static_cast<float>(e.mouseMove.x), static_cast<float>(e.mouseMove.y));
                break;
            case sf::Event::MouseButtonPressed:
                switch (e.key.code)
                {
                case sf::Mouse::Left:
                    global.leftMouseBtn = 1;
                    break;
                }
                break;

                //Keyboard Events
            case sf::Event::KeyPressed:
                if(screen == 's')
                {
                    switch (e.key.code)
                    {
                    case sf::Keyboard::Up:
                        Game.single.player1.upPressed();
                        break;
                    case sf::Keyboard::Down:
                        Game.single.player1.downPressed();
                        break;
                    case sf::Keyboard::Right:
                        Game.single.player1.rightPressed();
                        break;
                    case sf::Keyboard::Left:
                        Game.single.player1.leftPressed();
                        break;
                    }
                }
                else if(screen == 'm')
                {
                    switch (e.key.code)
                    {
                    case sf::Keyboard::Up:
                        Game.multi.player2.upPressed();
                        break;
                    case sf::Keyboard::Down:
                        Game.multi.player2.downPressed();
                        break;
                    case sf::Keyboard::Right:
                        Game.multi.player2.rightPressed();
                        break;
                    case sf::Keyboard::Left:
                        Game.multi.player2.leftPressed();
                        break;
                    case sf::Keyboard::W:
                        Game.multi.player1.upPressed();
                        break;
                    case sf::Keyboard::S:
                        Game.multi.player1.downPressed();
                        break;
                    case sf::Keyboard::D:
                        Game.multi.player1.rightPressed();
                        break;
                    case sf::Keyboard::A:
                        Game.multi.player1.leftPressed();
                        break;
                    }
                }
                break;
            case sf::Event::KeyReleased:
                if(screen == 's')
                {
                    switch (e.key.code)
                    {
                    case sf::Keyboard::Down:
                        Game.single.player1.downRealesed();
                        break;
                    case sf::Keyboard::Right:
                        Game.single.player1.rightRealesed();
                        break;
                    case sf::Keyboard::Left:
                        Game.single.player1.leftRealesed();
                        break;
                    }
                }
                else if(screen == 'm')
                {
                    switch (e.key.code)
                    {
                    case sf::Keyboard::Down:
                        Game.multi.player2.downRealesed();
                        break;
                    case sf::Keyboard::Right:
                        Game.multi.player2.rightRealesed();
                        break;
                    case sf::Keyboard::Left:
                        Game.multi.player2.leftRealesed();
                        break;
                    case sf::Keyboard::S:
                        Game.multi.player1.downRealesed();
                        break;
                    case sf::Keyboard::D:
                        Game.multi.player1.rightRealesed();
                        break;
                    case sf::Keyboard::A:
                        Game.multi.player1.leftRealesed();
                        break;
                    }
                }
                break;
            }
        }

        //Logic

        if (!global.GamePaused)
            switch (screen)
            {
            case 'h':
                main.Logic(screen, Game);
                break;
            case 's':
                Game.SingleLogic(screen);
                Game.PauseLogic(screen);
                break;
            case 'm':
                Game.MultiLogic(screen);
                Game.PauseLogic(screen);
                break;
            case 'c':
                credits.Logic(screen);
                break;
            case 'i':
                instructions.Logic(screen, Game.crntMode);
                break;
            }
        else pause.Logic(screen, Game);

        global.Logic();

        //Rendering
        window.clear();
        global.renderBG(window);

        switch (screen)
        {
        case 'h': //Default which is main menu (home)
            main.render(window);
            break;
        case 's': //Single Player
            Game.renderSingle(window);
            break;
        case 'm': //Multiplayer
            Game.renderMulti(window);
            break;
        case 'i': //Instructions
            instructions.render(window);
            break;
        case 'c': //Credits
            credits.render(window);
            break;
        }

        if (global.GamePaused) //Pause Menu to show above current screen
            pause.render(window);

        global.renderCursor(window);
        window.display();
    }

    return 0;
}