//Including Libraries
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>

//Constants
#define screenWidth 1000
#define screenHeight 650
#define groundTop 590

struct Global
{
    //Clicked inside the Game
    bool LeftClick=0;
    bool GamePaused=0;

    //Sounds
    sf::SoundBuffer btnHoverbufr, btnClickbufr;
    sf::Sound btnHover, btnClick;
    bool soundEnabled =1;
    
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
        cursor.setOrigin(cursorTexture.getSize().x / 7.0f, 4.0f);
        cursor.setScale(0.08f,0.08f);

        //Create Background
        backgroundTexture.loadFromFile("Data/Images/Background1.jpg");
        background.setTexture(backgroundTexture);

        //Load Fonts
        BtnFont.loadFromFile("Data/Fonts/fontBtn.ttf");
    }

    void Logic(sf::Vector2f& mousePos)
    {
        //Moving Cursor with mouse position
        cursor.setPosition(mousePos);
    }
    
    //Render Background
    void renderBG(sf::RenderWindow& window)
    {
        window.draw(background);
    }
    
    //Render Cursor
    void renderCursor(sf::RenderWindow& window)
    {
        window.draw(cursor);
    }

}global;

struct Gravity
{	
	bool inAir = 0;
	float dv=1.5f, maxVY =100.0f, lostE = 0.35f,groundFr = 0.25f;
	
    void activate(sf::Sprite& body,sf::Vector2f& bodyV)
	{
		inAir = body.getPosition().y + body.getGlobalBounds().height / 2 < groundTop; //If object is above Air
		
		if(inAir)
		{
			if(bodyV.y < maxVY) bodyV.y += dv;
		}
		else
		{
		 	bodyV.y = -bodyV.y + bodyV.y * lostE;
			bodyV.x -= bodyV.x * groundFr;
			body.setPosition(body.getPosition().x, groundTop - body.getGlobalBounds().height / 2 + 5);
		}
	}
};

struct Object
{
    struct Player
    {
        //VARIABLES

        bool up=0,down=0,right=0,left=0; //Movement Booleans
        const float lostE = 0.25f;
        //Character
        sf::Texture texture;
        sf::Sprite sprite;
        sf::Vector2f velocity;

        Gravity gravity; //chracter gravity
        //FUNCTIONS

        void create(std::string path,sf::Vector2f pos)
        {
            texture.loadFromFile(path);
            sprite.setTexture(texture);
            sprite.setPosition(pos);
            sprite.setOrigin(sprite.getGlobalBounds().width / 2,sprite.getGlobalBounds().height / 2);
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
            if(up)
            {
                velocity.y = -6.0f;
                up = 0;
            }
            else gravity.activate(sprite, velocity);

            if(down && currentBottomPos < groundTop) velocity.y = 5.0f;
            if(right && currentRightPos < screenWidth) velocity.x = 2.5f;
            if(left && currentLeftPos > 0) velocity.x = -2.5f;
            
            //Screen Boundries
            if(sprite.getGlobalBounds().left <= 0) //Left Boundries
            {
                sprite.setPosition(sprite.getGlobalBounds().width / 2,sprite.getPosition().y);
            }
            
            if(sprite.getGlobalBounds().left + sprite.getGlobalBounds().width >= screenWidth) //Right Boundries
            {
                sprite.setPosition(screenWidth - sprite.getGlobalBounds().width / 2,sprite.getPosition().y);
            }

            //Movement Action
            sprite.move(velocity);
        }

        bool stopCollision(sf::Sprite& body,sf::Vector2f& bodyV)
        {
            //Collisions
            if(sprite.getGlobalBounds().intersects(body.getGlobalBounds())) //Collision with body
            {
                bodyV = sf::Vector2f(velocity.x * 4, velocity.y * 3);
                velocity = -velocity;
                return true;
            }
            return false;
        }
        
        //Pressed button
        void upPressed()
        {
            if(sprite.getPosition().y + sprite.getGlobalBounds().height /2 >= groundTop) //Jump
                up=1;
        }

        void downPressed()
        {
            down=1;
        }

        void rightPressed()
        {
            right=1;
        }

        void leftPressed()
        {
            left=1;
        }

        //Released button

        void upRealesed()
        {
            up=0;
        }

        void downRealesed()
        {
            down=0;
        }

        void rightRealesed()
        {
            right=0;
        }

        void leftRealesed()
        {
            left=0;
        }
    };

    struct Ball
    {
        ////VARIABLES

        // Ball Propreties
        sf::Texture texture;
        sf::Sprite sprite;

        const float radius=25;

        // Physics
        Gravity gravity;
        sf::Vector2f velocity;

        void create()
        {
            texture.loadFromFile("Data/Images/ball.png");
            sprite.setTexture(texture);
            sprite.setOrigin(sf::Vector2f(25, 25));
            sprite.setPosition(sf::Vector2f(500, 100));
            sprite.setScale(0.80f,0.80f);
            gravity.dv = 0.5;
            gravity.groundFr=0.0f;
        }

        void move()
        {
            const float lostE = 0.25f;
            //Screen Boundries
            if(sprite.getGlobalBounds().left <= 0) //Left Boundries
            {
                sprite.setPosition(sprite.getGlobalBounds().width / 2,sprite.getPosition().y);
                velocity.x = -velocity.x + velocity.x * lostE;
            }
            if(sprite.getGlobalBounds().left + sprite.getGlobalBounds().width >= screenWidth) //Right Boundries
            {
                sprite.setPosition(screenWidth - sprite.getGlobalBounds().width / 2,sprite.getPosition().y);
                velocity.x = -velocity.x + velocity.x * lostE;
            }

            gravity.activate(sprite, velocity);

            sprite.move(velocity);
            sprite.rotate(velocity.x);
        }
    };    
};

struct Button
{
    
    struct Rectangular
    {
        sf::Texture frameTexture;
        sf::Vector2f size;
        sf::Sprite frame;
        sf::Text title;

        bool inside=0;

        void create(sf::Vector2f pos,std::string x)
        {
            //Button Style
            frameTexture.loadFromFile("Data/Images/RecButton.png");
            size = sf::Vector2f(static_cast<float>(frameTexture.getSize().x), static_cast<float>(frameTexture.getSize().y));
            size.x /= 3;
            frame.setTexture(frameTexture);
            notClicked();
            frame.setScale(1,0.5);
            frame.setOrigin(sf::Vector2f(size.x / 2.0f, size.y / 2.0f));
            frame.setPosition(pos);

            ////Text inside button
            title.setFont(global.BtnFont);
            title.setCharacterSize(50);
            title.setString(x);
            title.setOrigin(title.getLocalBounds().width /2, title.getGlobalBounds().height / 2);
            title.setPosition(frame.getPosition().x, frame.getPosition().y - 15);
        }

        void render(sf::RenderWindow& window)
        {
            window.draw(frame);
            window.draw(title);
        }

        void disabled()
        {
            frame.setTextureRect(sf::IntRect(static_cast<int>(size.x) * 2 + 30,0, static_cast<int>(size.x) - 28, static_cast<int>(size.y)));
        }
        void notClicked()
        {
            frame.setTextureRect(sf::IntRect(0,0, static_cast<int>(size.x) - 27, static_cast<int>(size.y)));
        }
        void clicked()
        {
            frame.setTextureRect(sf::IntRect(static_cast<int>(size.x) + 12,0, static_cast<int>(size.x) - 27, static_cast<int>(size.y)));
        }

    };

    //Menna//
    struct Round
    {
        sf::Texture Tex;
        sf::Vector2f size;
        sf::Sprite sprite;
        bool inside = 0;

        void disabled()
        {
            sprite.setTextureRect(sf::IntRect(static_cast<int>(size.x), 0, static_cast<int>(size.x), static_cast<int>(size.y)));
        }
        void notClicked()
        {
            sprite.setTextureRect(sf::IntRect(0, 0, static_cast<int>(size.x), static_cast<int>(size.y)));
        }
        void clicked()
        {
            sprite.setTextureRect(sf::IntRect(static_cast<int>(size.x), 0, static_cast<int>(size.x), static_cast<int>(size.y)));
        }

        void create(std::string Type)
        {
            Tex.loadFromFile("Data/Images/" + Type + ".png");
            
            sprite.setTexture(Tex);
            
            if(Type == "Cancel")
                sprite.setScale(0.15, 0.15);
            else
                sprite.setScale(0.4, 0.4);

            sprite.setOrigin(Tex.getSize().x / 6, Tex.getSize().y / 6);

            size = sf::Vector2f(static_cast<float>(Tex.getSize().x), static_cast<float>(Tex.getSize().y));
            size.x /= 3;
            notClicked();
        }
    };
    
};

struct Match
{
    ////VARIABLES

    // Textures declaration
    sf::Texture g1,g2;

    // Bodies declaration
    sf::Sprite goal1, goal2;
    sf::RectangleShape ground;
    Object::Player player1,player2;
    Object::Ball ball;
    
    //Sounds
    sf::SoundBuffer kickBallSoundbuff;
    sf::Sound kickBallSound;

    //Buttons
    Button::Round pauseBtn;

    ////FUNCTIONS

    void create()
    {
        // Players
        player1.create("Data/Images/Head1.png", sf::Vector2f(120, 550));
        player2.create("Data/Images/Head2.png", sf::Vector2f(880, 550));
        
        //Ball
        ball.create();
        
        //Goals
        g1.loadFromFile("Data/Images/Goal1.png");
        goal1.setTexture(g1);
        goal1.setOrigin(sf::Vector2f(50, 90));
        goal1.setPosition(sf::Vector2f(20, 500));
        
        g2.loadFromFile("Data/Images/Goal2.png");
        goal2.setTexture(g2);
        goal2.setOrigin(sf::Vector2f(50, 90));
        goal2.setPosition(sf::Vector2f(980, 500));

        //Sounds
        kickBallSoundbuff.loadFromFile("Data/Sounds/Kick.wav");
        kickBallSound.setBuffer(kickBallSoundbuff);

        //Pause Button
        pauseBtn.create("PauseButton");
        pauseBtn.sprite.setPosition(screenWidth - 100,10);
    } 
    
    //Logic

    void SingleLogic()
    {
        //Collisions
        if(player1.stopCollision(ball.sprite,ball.velocity) || player2.stopCollision(ball.sprite,ball.velocity))
            kickBallSound.play();

        player1.stopCollision(player2.sprite,player2.velocity);
        //Movement Control
        player1.move();
        player2.move();
        ball.move();
    }
    
    void PauseLogic(sf::Vector2f& mousePos, char& session)
    {
        if(pauseBtn.sprite.getGlobalBounds().contains(mousePos))
        {
            if (!pauseBtn.inside)
            {
                pauseBtn.clicked();
                global.btnHover.play();
                pauseBtn.inside = 1;
            }
            
            if (global.LeftClick)
            {
                global.btnClick.play();
                global.GamePaused=1;
            }
        }
        else
        {
            if (pauseBtn.inside)
                pauseBtn.notClicked();
            pauseBtn.inside = 0;
        }
    }

    void restart()
    {
        player1.sprite.setPosition(120,550);
        player2.sprite.setPosition(880, 550);
        ball.sprite.setPosition(500, 100);
    }

    //Rendering
    void render(sf::RenderWindow& window)
    {
        window.draw(ground);
        window.draw(ball.sprite);
        window.draw(player1.sprite);
        window.draw(player2.sprite);
        window.draw(goal1);
        window.draw(goal2);
        window.draw(pauseBtn.sprite);
    }
}Game;

struct Menu
{    
    struct Main
    {   
        /////////////////VARIABLES

        //Music
        sf::Music BGMusic;

        //Buttons
        static const int noOfBtns=4;
        Button::Rectangular btn[noOfBtns];
        std::string btnTitle[noOfBtns] ={"SinglePlayer", "MultiPlayer", "Instructions", "Credits"};
        char btnSession[noOfBtns] = {'s', 'm', 'i', 'c'};

        /////////////////FUNCTIONS

        //Creating Objects
        void create()
        {        
            //Create Buttons : Divided spaces in screen into 8 Xs and 7 Ys to put buttons in order
            for (char i = 0; i < 4; i++)
            {
                btn[i].create(sf::Vector2f(screenWidth /8 * 4, screenHeight / 6.5f * (i+3) ), btnTitle[i]);
            }

            //Load and Play Music
            BGMusic.openFromFile("Data/Sounds/MainMenu.wav");
            BGMusic.setLoop(true);
            BGMusic.play();
        }

        //Logic

        //When mouse hovers over buttons
        void Logic(sf::RenderWindow& window, char& session,sf::Vector2f& mousePos)
        {   
            //Buttons Hovered or Clicked Actions

            for (int i = 0; i < noOfBtns; i++)
            {
                if(btn[i].frame.getGlobalBounds().contains(mousePos))
                {
                    if (!btn[i].inside)
                    {
                        btn[i].clicked();
                        global.btnHover.play();
                        btn[i].inside = 1;
                    }
                    
                    if (global.LeftClick)
                    {
                        global.btnClick.play();
                        session = btnSession[i];
                        BGMusic.stop();
                    }
                    
                }
                else
                {
                    if (btn[i].inside)
                        btn[i].notClicked();
                    btn[i].inside = 0;
                    
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

    //Mariam//
    //credits//
    struct Credits
    {

        //credits photo
        sf::Texture creditstexture;
        sf::Sprite credits;

        //credits Background Music
        sf::Music BGMusic;
        bool isPlayed=0;

        //Return Button
        Button::Round cancelBtn;

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
            cancelBtn.Tex.loadFromFile("Data/Images/Cancel.png");
            cancelBtn.sprite.setTexture(cancelBtn.Tex);
        }
        
        void Logic()
        {
            if(!isPlayed)
            {
                BGMusic.play();
                isPlayed=1;
            }
        }

        //Rendering
        void render(sf::RenderWindow& window)
        {
            window.draw(credits);
        }

    };


    //Mariam//
    //instructions//
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
            returnBtn.sprite.setScale(0.25f,0.25f);
        }
        
        //Button Logic
        void Logic(sf::Vector2f& mousePos, char& session)
        {
            if(returnBtn.sprite.getGlobalBounds().contains(mousePos))
            {
                if (!returnBtn.inside)
                {
                    returnBtn.clicked();
                    global.btnHover.play();
                    returnBtn.inside = 1;
                }
                
                if (global.LeftClick)
                {
                    global.btnClick.play();
                    
                }
            }
            else
            {
                if (returnBtn.inside)
                    returnBtn.notClicked();
                returnBtn.inside = 0;
            }
        }

            //Rendering
            void render(sf::RenderWindow& window)
            {
                window.draw(instructions);
                window.draw(returnBtn.sprite);
            }

    };

    //Menna//
    struct Pause
    {
        // VARIABLES
        static const int n = 6;
        Button::Round btn[n];
        
        char btnsession[n] = { 'r','s','m','u','i','h' };
        /*
            r(return) Cancel
            s(single) Restart
            m(Mute) Mute Sounds
            u(Unmute) Unmute Sounds
            i(instructions)
            h(Home) Main Menu
        */
        
        //Background for Buttons
        sf::Texture bgT;
        sf::Sprite bg;
        
        //Semi Transparent Background
        sf::RectangleShape Blur;
        
        //Title
        sf::Text paused;
        sf::Text title;
        sf::String titleString[n] = {"Return","Restart","Mute Sounds", "Unmute Sounds","Instructions","Home" };

        // FUNCTIONS
        void create()
        {
            // Background
            bgT.loadFromFile("Data/Images/pause menu.png");
            bg.setTexture(bgT);
            bg.setOrigin(bgT.getSize().x / 2, bgT.getSize().y / 2);
            bg.setScale(1, 1.05);
            bg.setPosition(screenWidth / 2, screenHeight / 2 + 50);
            
            //Semi Transparent Background
            Blur.setSize(sf::Vector2f(screenWidth,screenHeight));
            Blur.setFillColor(sf::Color(220,220,220,80));


            // Buttons
            
            // 1-resume 2-restart 3-mute 4-unmute 5-instructions 6-main menu
            sf::String s[] = { "Cancel","Restart","Mute", "Unmute","Inst","Menu" };
            for (int i = 0; i < n; i++)
            {
                btn[i].create(s[i]);
            }

            btn[0].sprite.setPosition(bgT.getSize().x + bgT.getSize().x / 4 - 20, bgT.getSize().y - bgT.getSize().y / 4 + 20);
            btn[1].sprite.setPosition(screenWidth / 2 - 160 + 70, screenHeight / 2 - 70);
            btn[2].sprite.setPosition(screenWidth / 2 + 160 - 70, screenHeight / 2 - 70);
            btn[3].sprite.setPosition(screenWidth / 2 + 160 - 70, screenHeight / 2 - 70);
            btn[4].sprite.setPosition(screenWidth / 2 - 160 + 70, screenHeight / 2 + 70);
            btn[5].sprite.setPosition(screenWidth / 2 + 160 - 70, screenHeight / 2 + 70);

            //Text

            //Pause Menu Title
            paused.setFont(global.BtnFont);
            paused.setCharacterSize(100);
            paused.setString("PAUSED");
            paused.setOrigin(paused.getLocalBounds().width / 2, paused.getGlobalBounds().height / 2);
            paused.setPosition(screenWidth / 2, screenHeight / 2 - 180);

            //Current Button Title
            title.setFont(global.BtnFont);
            title.setOrigin(title.getGlobalBounds().width / 2, title.getGlobalBounds().height / 2);
            title.setPosition(screenWidth / 2.2f,screenHeight / 1.85f);
            title.setCharacterSize(25);
        }

        void Logic(sf::RenderWindow& window, char& session, sf::Vector2f& mousePos)
        {
            for (int i = 0; i < n; i++)
            {
                if (btn[i].sprite.getGlobalBounds().contains(mousePos))
                {
                    if (!btn[i].inside)
                    {
                        btn[i].clicked();
                        global.btnHover.play();
                        btn[i].inside = 1;
                        title.setString(titleString[i]);
                    }
                    
                    if (global.LeftClick)
                    {
                        global.btnClick.play();

                        switch (btnsession[i])
                        {
                        case 's':
                            Game.restart();
                        case 'r':
                            global.GamePaused=0;
                            session = 's';
                            break;
                        case 'm':
                            global.soundEnabled=0;
                            break;
                        case 'u':
                            global.soundEnabled=1;
                            break;
                        case 'i':
                            global.GamePaused=0;
                            session='i';
                            break;
                        case 'h':
                            global.GamePaused=0;
                            session='h';
                            break;
                        }
                    }
                }
                else
                {
                    if (btn[i].inside)
                    {
                        btn[i].notClicked();
                        title.setString("");
                    }
                    btn[i].inside = 0;
                }
            }
        }

        void render(sf::RenderWindow& window)
        {
            window.draw(Blur);
            window.draw(bg);
            window.draw(paused);

            for (int i = 0; i < n; i++)
            {
                window.draw(btn[i].sprite);
            }

            window.draw(title);
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
    title.setOrigin(title.getGlobalBounds().width / 2,title.getGlobalBounds().height / 2); 
    title.setPosition(screenWidth/2,screenHeight/2);
    window.clear(sf::Color::Black);
    window.draw(title);
    window.display();
}

int main()
{
    //variables
    char screen='h'; // to know which screen to render and handle
    /*
        h(Home) Main Menu
        s(Single) SinglePlayer
        l(Level Selection)
        c(Credits)
        m(Multiplayer)
        i(instructions)
        p(Pause)    
    */
    sf::Vector2f mousePos; //to save current mouse position

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
                    if(screen == 's')
                        global.GamePaused=1;
                    break;
                //Mouse Events
                case sf::Event::MouseMoved:
                    mousePos = sf::Vector2f(static_cast<float>(e.mouseMove.x), static_cast<float>(e.mouseMove.y));
                    break;
                case sf::Event::MouseButtonPressed:
                    switch (e.key.code)
                    {
                    case sf::Mouse::Left:
                        global.LeftClick=1;
                        break;
                    }
                    break;
                case sf::Event::MouseButtonReleased:
                    switch (e.key.code)
                    {
                    case sf::Mouse::Left:
                        global.LeftClick=0;
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
                    }
                    break;
            }
        }

        //Logic
        global.Logic(mousePos);

        if(!global.GamePaused)
            switch (screen)
            {
            case 'h':
                main.Logic(window, screen, mousePos);
                break;
            case 's':
                Game.SingleLogic();
                Game.PauseLogic(mousePos, screen);
                break;
            case 'c':
                credits.Logic();
                break;
            }
        else pause.Logic(window, screen, mousePos);
        //Rendering
        window.clear();
        global.renderBG(window);
        
        switch (screen)
        {
        case 'h': //Default which is main menu
            main.render(window);
            break;
        case 's': //Single Player
            Game.render(window);
            break;
        case 'm': //Multiplayer
            break;
        case 'i': //Instructions
            instructions.render(window);
            break;
        case 'c': //Credits
            credits.render(window);
            break;
        }

        if(global.GamePaused) //Pause Menu to show above current screen
            pause.render(window);

        global.renderCursor(window);
        window.display();
    }

    return 0;
}