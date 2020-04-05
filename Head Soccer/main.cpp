//Including Libraries
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>

//Constants
#define screenWidth 1000
#define screenHeight 650
#define groundTop 590

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

struct Player
{
    //VARIABLES

    bool up=0,down=0,right=0,left=0; //Movement Booleans
    const float lostE = 0.25f;
    //Character
    sf::Texture texture;
    sf::Sprite character;

    sf::Vector2f velocity; //Character current Velocity
    Gravity gravity; //chracter gravity
    //FUNCTIONS

    void create(std::string path,sf::Vector2f pos)
    {
        texture.loadFromFile(path);
        character.setTexture(texture);
        character.setPosition(pos);
        character.setOrigin(character.getGlobalBounds().width / 2,character.getGlobalBounds().height / 2);
        gravity.lostE = 0.9f;
        gravity.dv = 0.15f;
    }

    void move()
    {
        float currentTopPos = character.getGlobalBounds().top;
        float currentBottomPos = currentTopPos + character.getGlobalBounds().height;
        float currentLeftPos = character.getGlobalBounds().left; //character.getPosition().y - character.getGlobalBounds().width / 2;
        float currentRightPos = currentLeftPos + character.getGlobalBounds().width;

        //Controls
        if(up)
        {
			velocity.y = -6.0f;
            up = 0;
        }
		else gravity.activate(character,velocity);

        if(down && currentBottomPos < groundTop) velocity.y = 5.0f;
        if(right && currentRightPos < screenWidth) velocity.x = 2.5f;
        if(left && currentLeftPos > 0) velocity.x = -2.5f;
        
        //Screen Boundries
        if(character.getGlobalBounds().left <= 0) //Left Boundries
		{
			character.setPosition(character.getGlobalBounds().width / 2,character.getPosition().y);
		}
        
		if(character.getGlobalBounds().left + character.getGlobalBounds().width >= screenWidth) //Right Boundries
		{
			character.setPosition(screenWidth - character.getGlobalBounds().width / 2,character.getPosition().y);
		}

        //Movement Action
        character.move(velocity);
    }

    bool stopCollision(sf::Sprite& body,sf::Vector2f& bodyV)
    {
        //Collisions
        if(character.getGlobalBounds().intersects(body.getGlobalBounds())) //Collision with body
        {
            bodyV = sf::Vector2f(velocity.x * 2, (velocity.y + 5) * 3);
            velocity -= velocity;
            return true;
        }
        return false;
    }
    
    //pressed button
    void upPressed()
    {
        if(character.getPosition().y + character.getGlobalBounds().height /2 >= groundTop)
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

    // Ball Shape
    sf::Texture ballT; //Texture to hold image
    sf::Sprite ball; //sprite to load image
    const float radius=25;

    // Physics
    Gravity gravity;
    sf::Vector2f velocity;

    void create()
    {
        ballT.loadFromFile("Data/Images/ball.png");
        ball.setTexture(ballT);
        ball.setOrigin(sf::Vector2f(25, 25));
        ball.setPosition(sf::Vector2f(500, 100));

    }

    void move()
    {
        const float lostE = 0.25f;
        //Screen Boundries
        if(ball.getGlobalBounds().left <= 0) //Left Boundries
		{
			ball.setPosition(ball.getGlobalBounds().width / 2,ball.getPosition().y);
			velocity.x = -velocity.x + velocity.x * lostE;
		}
		if(ball.getGlobalBounds().left + ball.getGlobalBounds().width >= screenWidth) //Right Boundries
		{
			ball.setPosition(screenWidth - ball.getGlobalBounds().width / 2,ball.getPosition().y);
			velocity.x = -velocity.x + velocity.x * lostE;
		}

        gravity.activate(ball, velocity);

        ball.move(velocity);
    }
};

struct Button
{
    sf::Texture frameTexture;
    sf::Vector2f size;
    sf::Font BtnFont;
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
        BtnFont.loadFromFile("Data/Fonts/fontBtn.ttf");
        title.setFont(BtnFont);
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

    void lock()
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

struct Menus
{
    struct Main
    {   
        /////////////////VARIABLES
        
        //Music
        sf::Music menuMusic;

        //Sounds
        sf::SoundBuffer btnHoverbufr, btnClickbufr;
        sf::Sound btnHover, btnClick;

        //Buttons
        static const int noOfBtns=4;
        Button btn[noOfBtns];
        std::string btnTitle[noOfBtns] ={"Single Player", "Multiplayer", "Instructions", "Credits"};
        char btnSession[noOfBtns] = {'s', 'm', 'i', 'c'};
        //Cursor
        sf::Texture cursorTexture;
        sf::Sprite cursor;

        
        /////////////////FUNCTIONS

        //Creating Objects
        void create()
        {        
            //Create Cursor
            cursorTexture.loadFromFile("Data/Images/cursor.png");
            cursor.setTexture(cursorTexture);
            cursor.setOrigin(cursorTexture.getSize().x / 7.0f, 4.0f);
            cursor.setScale(0.08f,0.08f);

            //Create Buttons : Divided spaces in screen into 8 Xs and 7 Ys to put buttons in order
            for (char i = 0; i < 4; i++)
            {
                btn[i].create(sf::Vector2f(screenWidth /8 * 4, screenHeight / 6.5f * (i+3) ), btnTitle[i]);
            }

            //Load and Play Music
            menuMusic.openFromFile("Data/Sounds/MainMenu.wav");
            menuMusic.setLoop(true);
            menuMusic.play();

            //Load Sounds
            btnHoverbufr.loadFromFile("Data/Sounds/btnHover.wav");
            btnHover.setBuffer(btnHoverbufr);
            btnClickbufr.loadFromFile("Data/Sounds/btnClick.wav");
            btnClick.setBuffer(btnClickbufr);
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
                        btnHover.play();
                        btn[i].inside = 1;
                    }
                    
                    if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                    {
                        btnClick.play();
                        session = btnSession[i];
                        menuMusic.stop();
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

        //Moving Cursor with mouse position
        void moveCursor(sf::Vector2f& mousePos)
        {
            cursor.setPosition(mousePos);
        }

        //Render Cursor
        void renderCursor(sf::RenderWindow& window)
        {
            window.draw(cursor);
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
        sf::Music backgroundMusic;
        bool isPlayed=0;

        //functions
        //creating credits
        void create()
        {
            creditstexture.loadFromFile("Data/Images/credits.jpg");
            credits.setTexture(creditstexture);
            backgroundMusic.openFromFile("Data/Sounds/Credits.wav");
        }
        
        void Logic()
        {
            if(!isPlayed)
            {
                backgroundMusic.play();
                backgroundMusic.setLoop(1);
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


        //functions
        //creating credits
        void create()
        {
            instructionstexture.loadFromFile("Data/Images/instructions.jpg");
            instructions.setTexture(instructionstexture);

        }

        //Rendering
        void render(sf::RenderWindow& window)
        {
            window.draw(instructions);

        }

    };
};

struct rButton
{
    sf::Texture Tex;
    sf::Vector2f size;
    sf::Sprite sprite;
    sf::SoundBuffer btnHoverbufr, btnClickbufr;
    sf::Sound btnHover, btnClick;
    bool inside = 0;
    void lock()
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
    void create()
    {
        size = sf::Vector2f(static_cast<float>(Tex.getSize().x), static_cast<float>(Tex.getSize().y));
        size.x /= 3;
        notClicked();
        btnHoverbufr.loadFromFile("Data/Sounds/btnHover.wav");
        btnHover.setBuffer(btnHoverbufr);
        btnClickbufr.loadFromFile("Data/Sounds/btnClick.wav");
        btnClick.setBuffer(btnClickbufr);
    }
};
struct PauseMenu
{
    // VARIABLES
    static const int n = 5;
    rButton btn[n];
    char btnsession[n] = { 'r','s','m','i','h' };

    sf::Texture bgT;
    sf::Sprite bg;
    sf::Font font;
    sf::Text paused;

    //Cursor
    sf::Texture cursorTexture;
    sf::Sprite cursor;

    //Sounds
    sf::SoundBuffer btnHoverbufr, btnClickbufr;
    sf::Sound btnHover, btnClick;

    // FUNCTIONS
    void create()
    {
        // 1-resume 2-restart 3-mute 4-instructions 5-main menu
        sf::String s[5] = { "Cancel.png","restart.png","mute.png","inst.png","Menu.png" };
        for (int i = 0; i < n; i++)
        {
            btn[i].Tex.loadFromFile("Data/Images/" + s[i]);
            btn[i].sprite.setTexture(btn[i].Tex);
            btn[i].sprite.setScale(0.4, 0.4);
            btn[i].sprite.setOrigin(btn[i].Tex.getSize().x / 6, btn[i].Tex.getSize().y / 6);
            btn[i].create();
        }
        // Buttons

        btn[1].sprite.setPosition(screenWidth / 2 - 160 + 70, screenHeight / 2 - 70);
        btn[2].sprite.setPosition(screenWidth / 2 + 160 - 70, screenHeight / 2 - 70);
        btn[3].sprite.setPosition(screenWidth / 2 - 160 + 70, screenHeight / 2 + 70);
        btn[4].sprite.setPosition(screenWidth / 2 + 160 - 70, screenHeight / 2 + 70);

        // Background and text
        bgT.loadFromFile("Data/Images/pause menu.png");
        bg.setTexture(bgT);
        bg.setOrigin(bgT.getSize().x / 2, bgT.getSize().y / 2);
        bg.setScale(1, 1.05);
        bg.setPosition(screenWidth / 2, screenHeight / 2 + 50);
        btn[0].sprite.setPosition(bgT.getSize().x + bgT.getSize().x / 4 - 20, bgT.getSize().y - bgT.getSize().y / 4 + 20);
        btn[0].sprite.setScale(0.15, 0.15);

        font.loadFromFile("Data/Fonts/fontBtn.ttf");
        paused.setFont(font);
        paused.setCharacterSize(100);
        paused.setString("PAUSED!");
        paused.setOrigin(paused.getLocalBounds().width / 2, paused.getGlobalBounds().height / 2);
        paused.setPosition(screenWidth / 2, screenHeight / 2 - 180);

        //Load Sounds
        btnHoverbufr.loadFromFile("Data/Sounds/btnHover.wav");
        btnHover.setBuffer(btnHoverbufr);
        btnClickbufr.loadFromFile("Data/Sounds/btnClick.wav");
        btnClick.setBuffer(btnClickbufr);

        //Create Cursor
        cursorTexture.loadFromFile("Data/Images/cursor.png");
        cursor.setTexture(cursorTexture);
        cursor.setOrigin(cursorTexture.getSize().x / 7.0f, 4.0f);
        cursor.setScale(0.08f, 0.08f);
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
                    btnHover.play();
                    btn[i].inside = 1;
                }
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    btnClick.play();
                    session = btnsession[i];
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
    //Moving Cursor with mouse position
    void moveCursor(sf::Vector2f& mousePos)
    {
        cursor.setPosition(mousePos);
    }

    //Render Cursor
    void renderCursor(sf::RenderWindow& window)
    {
        window.draw(cursor);
    }
    void render(sf::RenderWindow& window)
    {
        window.draw(bg);
        window.draw(paused);
        for (int i = 0; i < n; i++)
        {
            window.draw(btn[i].sprite);
        }
    }
};

struct Match
{
    ////VARIABLES

    // Textures declaration
    sf::Texture g1,g2;

    // Bodies declaration
    sf::Sprite goal1, goal2;
    sf::RectangleShape ground;
    Player player1,player2;
    Ball ball;
    
    //Sounds
    sf::SoundBuffer kickBallSoundbuff;
    sf::Sound kickBallSound;

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

    }   
    
    //Logic

    void SingleLogic()
    {
        //Movement Control
        player1.move();
        player2.move();
        ball.move();

        //Collisions
        if(player1.stopCollision(ball.ball,ball.velocity) || player2.stopCollision(ball.ball,ball.velocity))
            kickBallSound.play();

        player1.stopCollision(player2.character,player2.velocity);
    }
    
    //Rendering
    void render(sf::RenderWindow& window)
    {
        window.draw(ground);
        window.draw(ball.ball);
        window.draw(player1.character);
        window.draw(player2.character);
        window.draw(goal1);
        window.draw(goal2);
    }
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
    char session='d'; // to know which screen to render and handle
    sf::Vector2f mousePos; //to save current mouse position

    //Creating window
    sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Head Soccer", sf::Style::Close | sf::Style::Titlebar);
    window.setFramerateLimit(60);
    window.setMouseCursorVisible(false);
    
    //Loading Screen till Resources Loads
    loadScreen(window);

    //Background
    sf::Texture backgroundTexture;
    sf::Sprite background;
    
    //Create Background
    backgroundTexture.loadFromFile("Data/Images/Background1.jpg");
    background.setTexture(backgroundTexture);
    
    //Main Menu
    Menus::Main main;
    main.create();


    //Credits//mariam//
    Menus::Credits credits;
    credits.create();

    //Instructions//mariam//
    Menus::Instructions instructions;
    instructions.create();

    //Single Player Session
    Match Game;
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
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::MouseMoved:
                    mousePos = sf::Vector2f(static_cast<float>(e.mouseMove.x), static_cast<float>(e.mouseMove.y));
                    break;
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
        switch (session)
        {
        case 'd':
            main.Logic(window, session, mousePos);
            break;
        case 's':
            Game.SingleLogic();
            break;
        case 'c':
            credits.Logic();
            break;
        }

        main.moveCursor(mousePos); //Move Cursor with the mouse

        //Rendering
        window.clear();
        window.draw(background);
        
        switch (session)
        {
        case 'd': //Default which is main menu
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
        
        main.renderCursor(window); //Rendering Game Cursor;
        window.display();
    }

    return 0;
}