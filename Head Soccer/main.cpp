//Including Libraries
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>

//Constants
#define screenWidth 1000
#define screenHeight 650
#define groundTop 590

/// Global Struct, contains stuff that are used in more than one scope
struct Global
{
    //if Mouse Left Button Clicked inside the Game
    bool leftMouseBtn = 0;

    //if Game is Paused
    bool GamePaused = 0;

    //if to return from Instructions to Game
    bool gotoPauseMenu = 0;

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
    bool inAir = 0;
    float dv = 1.5f, maxVY = 100.0f, lostE = 0.35f, groundFr = 0.25f;

    void activate(sf::Sprite& body, sf::Vector2f& bodyV)
    {
        inAir = body.getPosition().y + body.getGlobalBounds().height / 2 < groundTop; //If object is above Air

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

        //Animation
        bool LPlyr = 1;
        int width = 0;
        int height = 0;
        int imgCnt = 0;

        Gravity gravity; //chracter gravity
        //FUNCTIONS

        void create(std::string path, sf::Vector2f pos,bool LeftPlayer)
        {
            LPlyr = LeftPlayer;
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
            gravity.lostE = 0.9f;
            gravity.dv = 0.15f;
        }

        void move()
        {
            float currentTopPos = sprite.getGlobalBounds().top;
            float currentBottomPos = currentTopPos + sprite.getGlobalBounds().height;
            float currentLeftPos = sprite.getGlobalBounds().left;
            float currentRightPos = currentLeftPos + sprite.getGlobalBounds().width;

            //Controls
            if (up)
            {
                velocity.y = -6.0f;
                up = 0;
            }
            else gravity.activate(sprite, velocity);

            // if(down && currentBottomPos < groundTop) velocity.y = 5.0f;
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

        bool ballCollision(sf::Sprite& body, sf::Vector2f& bodyV)
        {
            bool atRight = velocity.x >= 2.5f && body.getPosition().x > sprite.getPosition().x;
            bool atLeft = velocity.x <= -2.5f && body.getPosition().x < sprite.getPosition().x;
            bool atAbove = body.getPosition().y < sprite.getPosition().y;
            bool atBottom = body.getPosition().y > sprite.getPosition().y;

            if (sprite.getGlobalBounds().intersects(body.getGlobalBounds())) //Collision Detection
            {
                if (atLeft) //Ball is at Left of the Player
                {
                    body.setPosition(sprite.getPosition().x - (sprite.getGlobalBounds().width - 5), body.getPosition().y);
                    bodyV.x = velocity.x * 3;
                }
                else if (atRight) //Ball is at Right of the Player
                {
                    body.setPosition(sprite.getPosition().x + (sprite.getGlobalBounds().width - 5), body.getPosition().y);
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
                    else bodyV.y = -bodyV.y + bodyV.y * 0.7f;
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

            return false;
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
    };

    struct Ball
    {
        ////VARIABLES

        // Ball Propreties
        sf::Texture texture;
        sf::Sprite sprite;

        const float radius = 25;
        const float lostE = 0.25f;
        // Physics
        Gravity gravity;
        sf::Vector2f velocity;

        void create()
        {
            texture.loadFromFile("Data/Images/ball.png");
            sprite.setTexture(texture);
            sprite.setOrigin(sf::Vector2f(25, 25));
            sprite.setPosition(sf::Vector2f(500, 100));
            sprite.setScale(0.80f, 0.80f);
            gravity.dv = 0.5;
            gravity.groundFr = 0.0f;
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

            gravity.activate(sprite, velocity);

            sprite.move(velocity);
            sprite.rotate(velocity.x);
        }

        void goalCollision(sf::Sprite& body)
        {
            sf::FloatRect goalTop = { body.getGlobalBounds().left, body.getGlobalBounds().top, body.getGlobalBounds().width, 20 };
            if (goalTop.intersects(sprite.getGlobalBounds()))
                velocity.y = -velocity.y + velocity.y * lostE;
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
        }

        //Checks if Mouse Hovered button
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
    bool LevelEnded = 0;
    std::string player[4] = { "Data/Images/BossBabySheet.png",  "Data/Images/ButterSheet.png", "Data/Images/GumballSheet.png", "Data/Images/MegaSheet.png" };
    std::string enemy[4] = { "Data/Images/TimSheet.png", "Data/Images/MojoSheet.png", "Data/Images/RobSheet.png", "Data/Images/MetroSheet.png" };
    int crntLvl = 3;
    //Level text
    sf::Font font;
    sf::Text text;

    //Functions

    void create()
    {

        font.loadFromFile("Data/Fonts/fontBtn.ttf");
        text.setFont(font);
        sf::String s = "Level " + std::to_string(crntLvl + 1);;
        text.setString(s);
        text.setCharacterSize(50);
        text.setOrigin(text.getGlobalBounds().width / 2, text.getGlobalBounds().height / 2);
        text.setPosition(screenWidth / 2, 50);
        text.setFillColor(sf::Color::Black);
        text.setStyle(sf::Text::Bold);


    };


    void logic()
    {

        if (crntLvl < 4)
        {

            if (LevelEnded)
            {
                crntLvl++;
                LevelEnded = 0;
            }
        }
    };


    void render(sf::RenderWindow& window)
    {
        window.draw(text);
    };
};



/// Match Struct, contains the main game screen
struct Match
{
    //// VARIABLES ////

        // Textures declaration
    sf::Texture gl;

    // Bodies declaration
    sf::Sprite goal1, goal2;
    sf::RectangleShape ground;
    Object::Player player1, player2;
    Object::Ball ball;

    // Sounds
    sf::SoundBuffer kickBallSoundbuff;
    sf::Sound kickBallSound;
    sf::SoundBuffer GameEndSoundBuff;
    sf::Sound GameEndSound;

    // Buttons
    Button::Round pauseBtn;

    // Scores
    sf::Text score1, score2;
    int Score1 = 0, Score2 = 0;
    bool inside_goal = 0, outside_goal = 0;
    sf::Text winOrlose;

    // Timer
    int timer = 3600;
    sf::Text timer_cnt;
    bool end = 0, time_finished = 0;

    //Levels
    Levels levels;

    //// FUNCTIONS ////

    void create()
    {
        // Players
        player1.create((levels.player[levels.crntLvl]), sf::Vector2f(120, 550), 1);
        player2.create((levels.enemy[levels.crntLvl]), sf::Vector2f(880, 550), 0);

        //Ball
        ball.create();

        //Goals
        gl.loadFromFile("Data/Images/Goal.png");
        goal1.setTexture(gl);
        goal1.setOrigin(sf::Vector2f(50, 90));
        goal1.setPosition(sf::Vector2f(20, 500));

        goal2.setTexture(gl);
        goal2.setTextureRect(sf::IntRect(gl.getSize().x, 0, -1 * gl.getSize().x, gl.getSize().y));
        goal2.setOrigin(sf::Vector2f(50, 90));
        goal2.setPosition(sf::Vector2f(980, 500));

        //Sounds
        kickBallSoundbuff.loadFromFile("Data/Sounds/Kick.wav");
        kickBallSound.setBuffer(kickBallSoundbuff);
        GameEndSoundBuff.loadFromFile("Data/Sounds/gEnd.wav");
        GameEndSound.setBuffer(GameEndSoundBuff);

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

        winOrlose.setFont(global.BtnFont);
        winOrlose.setCharacterSize(60);
        winOrlose.setPosition(screenWidth / 2, screenHeight / 2);

        //level text
        levels.create();
    }

    // Logic

    void SingleLogic(char& screen)
    {
        // Collisions
        if (player1.ballCollision(ball.sprite, ball.velocity) && global.soundEnabled)// || player2.stopCollision(ball.sprite,ball.velocity))
            kickBallSound.play();

        ball.goalCollision(goal1);
        ball.goalCollision(goal2);

        // Scoring
        if (!(ball.sprite.getGlobalBounds().intersects(goal1.getGlobalBounds()) ||
            ball.sprite.getGlobalBounds().intersects(goal2.getGlobalBounds())))
            outside_goal = 1;
        if (!inside_goal && outside_goal && ball.sprite.getGlobalBounds().intersects(goal1.getGlobalBounds()))
            Score2++, inside_goal = 1, outside_goal = 0;
        else if (!inside_goal && outside_goal && ball.sprite.getGlobalBounds().intersects(goal2.getGlobalBounds()))
            Score1++, inside_goal = 1, outside_goal = 0;
        else
            inside_goal = 0;

        // End of Game
        if (timer <= 0)
        {
            if (Score1 != Score2 && !end)
            {
                GameEndSound.play();
                end = 1;
                ball.velocity = {};
                player1.velocity = {};
                player2.velocity = {};
                winOrlose.setString(Score1 > Score2 ? "YOU WIN" : "YOU LOSE");
            }
            time_finished = 1;
        }

        // Movement Control
        player1.move();
        ball.move();

        if (!time_finished)
            timer--;

        //Levels iteration
        if (Score1 > Score2&& end&& levels.crntLvl < 4)
        {
            levels.LevelEnded = 1;
            levels.logic();
            create();
            player1.move();
            ball.move();
            Score1 = 0;
            Score2 = 0;
            timer = 3600;
            end = time_finished = 0;

        }

        //Returning to main menu after finshing all levels
        if (levels.crntLvl == 4)
        {
            screen = 'h';
            levels.crntLvl = 0;
            create();
        }
    }

    void MultiLogic()
    {
        // Collisions
        if ((player1.ballCollision(ball.sprite, ball.velocity) || player2.ballCollision(ball.sprite, ball.velocity)) && global.soundEnabled)// || player2.stopCollision(ball.sprite,ball.velocity))
            kickBallSound.play();

        ball.goalCollision(goal1);
        ball.goalCollision(goal2);

        // Scoring
        if (!(ball.sprite.getGlobalBounds().intersects(goal1.getGlobalBounds()) ||
            ball.sprite.getGlobalBounds().intersects(goal2.getGlobalBounds())))
            outside_goal = 1;
        if (!inside_goal && outside_goal && ball.sprite.getGlobalBounds().intersects(goal1.getGlobalBounds()))
            Score2++, inside_goal = 1, outside_goal = 0;
        else if (!inside_goal && outside_goal && ball.sprite.getGlobalBounds().intersects(goal2.getGlobalBounds()))
            Score1++, inside_goal = 1, outside_goal = 0;
        else
            inside_goal = 0;

        // Movement Control
        player1.move();
        player2.move();
        ball.move();

        // End of Game
        if (timer <= 0)
        {
            if (Score1 != Score2 && !end)
            {
                GameEndSound.play();
                end = 1;
                ball.velocity = {};
                player1.velocity = {};
                player2.velocity = {};
                winOrlose.setString(Score1 > Score2 ? "YOU WIN" : "YOU LOSE");
            }
            time_finished = 1;
        }

        if (!time_finished)
            timer--;
    }

    void PauseLogic(char& session)
    {
        if (pauseBtn.mouseLeftClicked())
            global.GamePaused = 1;
    }

    void restart()
    {
        player1.sprite.setPosition(120, 550);
        player2.sprite.setPosition(880, 550);
        ball.sprite.setPosition(500, 100);

        player1.velocity = {};
        player2.velocity = {};
        ball.velocity = {};
        end = timer = time_finished = 0;
        Score1 = Score2 = 0;
        timer = 3600;
    }

    // Rendering
    void render(sf::RenderWindow& window)
    {
        levels.render(window);
        window.draw(ground);
        window.draw(ball.sprite);
        window.draw(player1.sprite);
        window.draw(player2.sprite);
        window.draw(goal1);
        window.draw(goal2);
        window.draw(pauseBtn.sprite);
        score1.setString(std::to_string(Score1));
        score2.setString(std::to_string(Score2));
        window.draw(score1);
        window.draw(score2);
        timer_cnt.setString(std::to_string(timer / 60) + ":0" + std::to_string(timer >= 0 ? (timer / 6) % 10 : 9 - (-timer / 6) % 10));
        window.draw(timer_cnt);
        if (end)
        {
            window.draw(global.background);
            window.draw(winOrlose);
        }

    }
}Game;

/// Menu Struct, contains all different menus of the game
struct Menu
{
    /// Main Menu, (Added by Tawfik)
    struct Main
    {
        /////////////////VARIABLES

        //Music
        sf::Music BGMusic;
        bool isPlaying = 0;

        //Buttons
        static const int noOfBtns = 4;
        Button::Rectangular btn[noOfBtns];
        std::string btnTitle[noOfBtns] = { "SinglePlayer", "MultiPlayer", "Instructions", "Credits" };
        char btnSession[noOfBtns] = { 's', 'm', 'i', 'c' };

        /////////////////FUNCTIONS

        //Creating Objects
        void create()
        {
            //Create Buttons : Divided spaces in screen into 8 Xs and 7 Ys to put buttons in order
            for (char i = 0; i < 4; i++)
            {
                btn[i].create(sf::Vector2f(screenWidth / 8 * 4, screenHeight / 6.5f * (i + 3)), btnTitle[i]);
            }

            //Load and Play Music
            BGMusic.openFromFile("Data/Sounds/MainMenu.wav");
            BGMusic.setLoop(true);
        }

        //Logic

        //When mouse hovers over buttons
        void Logic(char& session)
        {
            //Play Music
            if (!isPlaying && global.soundEnabled)
            {
                BGMusic.play();
                isPlaying = 1;
            }

            //Buttons Hovered or Clicked Actions
            for (int i = 0; i < noOfBtns; i++)
            {
                if (btn[i].mouseLeftClicked())
                {
                    session = btnSession[i];
                    BGMusic.stop();
                    isPlaying = 0;
                }
            }
        }

        //Rendering
        void render(sf::RenderWindow& window)
        {
            for (int i = 0; i < noOfBtns; i++)
            {
                btn[i].render(window);
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
        void Logic(char& session)
        {
            if (returnBtn.mouseLeftClicked())
            {
                if (global.gotoPauseMenu)
                {
                    global.GamePaused = 1;
                    session = 's';
                    global.gotoPauseMenu = 0;
                }
                else
                {
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
        static const int n = 6;
        Button::Round btn[n];

        //Background for Buttons
        sf::Texture bgT;
        sf::Sprite bg;

        //Semi Transparent Background
        sf::RectangleShape Blur;

        //Title
        sf::Text paused;

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

            // 1-resume 2-restart 3-mute 4-unmute 5-instructions 6-main menu
            sf::String s[] = { "Cancel","Restart","Mute", "Unmute","Inst","Home" };
            for (int i = 0; i < n; i++)
            {
                btn[i].create(s[i]);
            }

            btn[0].sprite.setPosition(bgT.getSize().x + bgT.getSize().x / 4.0f - 20.0f, bgT.getSize().y - bgT.getSize().y / 4.0f + 20.0f); //Cancel Button
            btn[1].sprite.setPosition(screenWidth / 2 - 160 + 70, screenHeight / 2 - 70); //Restart Button
            btn[2].sprite.setPosition(screenWidth / 2 + 160 - 70, screenHeight / 2 - 70); //Mute Button
            btn[3].sprite.setPosition(screenWidth / 2 + 160 - 70, screenHeight / 2 - 70); //Unmute Button
            btn[3].sprite.setScale(0, 0);
            btn[4].sprite.setPosition(screenWidth / 2 - 160 + 70, screenHeight / 2 + 70); //Instructions Button
            btn[5].sprite.setPosition(screenWidth / 2 + 160 - 70, screenHeight / 2 + 70); //Home Button

            //Text

            //Pause Menu Title
            paused.setFont(global.BtnFont);
            paused.setCharacterSize(100);
            paused.setString("PAUSED");
            paused.setOrigin(paused.getLocalBounds().width / 2, paused.getGlobalBounds().height / 2);
            paused.setPosition(screenWidth / 2, screenHeight / 2 - 180);
        }

        void Logic(char& session)
        {
            for (int i = 0; i < n; i++)
            {
                if (btn[i].mouseLeftClicked())
                {
                    switch (i)
                    {
                    case 1: //Restart
                        Game.restart();
                    case 0: //Cancel
                        global.GamePaused = 0;
                        session = 's';
                        break;
                    case 2: //Mute
                        global.soundEnabled = 0;
                        btn[2].sprite.setScale(0, 0);
                        btn[3].sprite.setScale(0.4f, 0.4f);
                        break;
                    case 3: //Unmute
                        global.soundEnabled = 1;
                        btn[2].sprite.setScale(0.4f, 0.4f);
                        btn[3].sprite.setScale(0, 0);
                        break;
                    case 4: //Instructions
                        global.gotoPauseMenu = 1;
                        global.GamePaused = 0;
                        session = 'i';
                        break;
                    case 5: //Home
                        global.GamePaused = 0;
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

int main()
{
    //variables
    char screen = 'h'; // to know which screen to render and handle

    /*
        h(Home) Main Menu
        s(Single) SinglePlayer
        l(Level Selection)
        c(Credits)
        m(Multiplayer)
        i(instructions)
        p(Pause)
    */

    //Creating window
    sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Head Soccer", sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(60);
    window.setMouseCursorVisible(false);

    //Loading Screen till Resources Loads
    loadScreen(window);

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
    Game.create();

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
                switch (e.key.code)
                {
                case sf::Keyboard::Up:
                    Game.player1.upPressed();
                    break;
                case sf::Keyboard::Down:
                    Game.player1.downPressed();
                    break;
                case sf::Keyboard::Right:
                    Game.player1.rightPressed();
                    break;
                case sf::Keyboard::Left:
                    Game.player1.leftPressed();
                    break;
                case sf::Keyboard::W:
                    Game.player2.upPressed();
                    break;
                case sf::Keyboard::S:
                    Game.player2.downPressed();
                    break;
                case sf::Keyboard::D:
                    Game.player2.rightPressed();
                    break;
                case sf::Keyboard::A:
                    Game.player2.leftPressed();
                    break;
                }
                break;

            case sf::Event::KeyReleased:
                switch (e.key.code)
                {
                case sf::Keyboard::Down:
                    Game.player1.downRealesed();
                    break;
                case sf::Keyboard::Right:
                    Game.player1.rightRealesed();
                    break;
                case sf::Keyboard::Left:
                    Game.player1.leftRealesed();
                    break;
                case sf::Keyboard::S:
                    Game.player2.downRealesed();
                    break;
                case sf::Keyboard::D:
                    Game.player2.rightRealesed();
                    break;
                case sf::Keyboard::A:
                    Game.player2.leftRealesed();
                    break;
                }
                break;
            }
        }

        //Logic

        if (!global.GamePaused)
            switch (screen)
            {
            case 'h':
                main.Logic(screen);
                break;
            case 's':
                Game.SingleLogic(screen);
                Game.PauseLogic(screen);
                break;
            case 'm':
                Game.MultiLogic();
                Game.PauseLogic(screen);
                break;
            case 'c':
                credits.Logic(screen);
                break;
            case 'i':
                instructions.Logic(screen);
                break;
            }
        else pause.Logic(screen);

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
            Game.render(window);
            break;
        case 'm': //Multiplayer
            Game.render(window);
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