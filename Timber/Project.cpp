// ============================================================
//  TIMBER - Practica dels 9 reptes
//  Base: TimberSFML3.cpp  (SFML 3)
// ============================================================
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <sstream>
#include <fstream>
#include <ctime>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <cassert>
#include <cstdlib>
#include <optional>

using namespace sf;

// ---------------- Constants del joc ----------------
const int   WINDOW_WIDTH   = 1920;
const int   WINDOW_HEIGHT  = 1080;
const int   NUM_BRANCHES   = 6;
const int   NUM_TREES      = 7;          // Repte 3: bosc d'arbres
const float timeBarHeight  = 80;
const float timeBarStartWidth = 400;
const float AXE_POSITION_LEFT  = 700;
const float AXE_POSITION_RIGHT = 1075;
const int   MAX_LIVES      = 3;          // Repte 4

// Enumeracio del costat de l'arbre
enum class Side { LEFT, RIGHT, NONE };

// Repte 7: modes de joc
enum class GameMode { FACIL, DIFICIL };

// ============================================================
//  Repte 9: excepcio propia per a la carrega de recursos
// ============================================================
class ResourceLoadException : public std::runtime_error {
public:
    explicit ResourceLoadException(const std::string& msg)
        : std::runtime_error(msg) {}
};

// ============================================================
//  Repte 1 + 9: carrega segura de recursos
//  Comprova el valor de retorn i atura l'execucio de forma controlada
// ============================================================
void loadTexture(Texture& texture, const std::string& path) {
    if (!texture.loadFromFile(path)) {
        throw ResourceLoadException("No s'ha pogut carregar la textura: " + path);
    }
    std::cout << "[OK] Textura carregada: " << path << std::endl;
}
void loadSound(SoundBuffer& buffer, const std::string& path) {
    if (!buffer.loadFromFile(path)) {
        throw ResourceLoadException("No s'ha pogut carregar el so: " + path);
    }
    std::cout << "[OK] So carregat: " << path << std::endl;
}

// ============================================================
//  Repte 2: estructura NPC (abelles / nuvols)
// ============================================================
struct NPC {
    Sprite sprite;
    bool  active;
    float speed;
    int   maxHeight;
    int   maxSpeed;
    int   sentit;            // 1 = dreta, -1 = esquerra
    float posicioInicialX;
    NPC(Texture& texture, int maxHeight_, int maxSpeed_, int sentit_, float posicioInicialX_)
        : sprite(texture), active(false), speed(0),
          maxHeight(maxHeight_), maxSpeed(maxSpeed_),
          sentit(sentit_), posicioInicialX(posicioInicialX_) {}
};

// ============================================================
//  Repte 3: estructura Tree (bosc d'arbres)
// ============================================================
struct Tree {
    Sprite sprite;
    int    layer;            // 0 = darrere, 1 = davant
    Tree(Texture& texture, int layer_) : sprite(texture), layer(layer_) {}
};

// ============================================================
//  Repte 6: estructura Branch (un sol vector d'estructures)
// ============================================================
struct Branch {
    Side   side;
    Sprite sprite;
    Branch(Texture& texture) : side(Side::NONE), sprite(texture) {}
};

// ---------------- Prototips ----------------
void updateNPC(NPC&, float);
void updateBranches(std::vector<Branch>&, int seed);
void updateBranchSprites(std::vector<Branch>&);
float timeForMode(GameMode);
int  readRecord(const std::string&);
void writeRecord(const std::string&, int);

// ============================================================
//  MAIN
// ============================================================
int main()
{
    VideoMode vm({ WINDOW_WIDTH, WINDOW_HEIGHT });
    RenderWindow window(vm, "Timber!!!", State::Windowed);

    // ---- Repte 1 + 9: carrega de recursos amb control d'errors ----
    Texture textureBackground, textureTree, textureTree2, textureRIP,
            texturePlayer, textureAxe, textureLog, textureBee,
            textureCloud, textureBranch;
    SoundBuffer chopBuffer, deathBuffer, timeBuffer;
    Font font;
    try {
        loadTexture(textureBackground, "graphics/background.png");
        loadTexture(textureTree,       "graphics/tree.png");
        loadTexture(textureTree2,      "graphics/tree2.png");
        loadTexture(textureRIP,        "graphics/rip.png");
        loadTexture(texturePlayer,     "graphics/player.png");
        loadTexture(textureAxe,        "graphics/axe.png");
        loadTexture(textureLog,        "graphics/log.png");
        loadTexture(textureBee,        "graphics/bee.png");
        loadTexture(textureCloud,      "graphics/cloud.png");
        loadTexture(textureBranch,     "graphics/branch.png");
        loadSound(chopBuffer,  "sound/chop.wav");
        loadSound(deathBuffer, "sound/death.wav");
        loadSound(timeBuffer,  "sound/out_of_time.wav");
        if (!font.openFromFile("fonts/KOMIKAP_.ttf"))
            throw ResourceLoadException("No s'ha pogut carregar la font: fonts/KOMIKAP_.ttf");
        std::cout << "[OK] Font carregada: fonts/KOMIKAP_.ttf" << std::endl;
    }
    catch (const ResourceLoadException& e) {
        std::cerr << "ERROR DE RECURS: " << e.what() << std::endl;
        return 1;   // aturada controlada
    }

    // ---- Sprites basics ----
    Sprite spriteBackground(textureBackground);
    spriteBackground.setPosition({ 0, 0 });

    Sprite spriteTree(textureTree);
    spriteTree.setPosition({ 810, 0 });

    Sprite spriteRIP(textureRIP);
    spriteRIP.setPosition({ 675, 2000 });

    Sprite spritePlayer(texturePlayer);
    spritePlayer.setPosition({ 675, 720 });

    Sprite spriteAxe(textureAxe);
    spriteAxe.setPosition({ 2000, 840 });

    Sprite spriteLog(textureLog);
    spriteLog.setPosition({ 810, 720 });
    bool  logActive = false;
    float logSpeedX = 1000;
    float logSpeedY = -1500;

    // ---- Repte 2: mes NPC decoratius (2 abelles + 4 nuvols) ----
    std::vector<NPC> NPCTable;
    NPCTable.push_back(NPC(textureBee,   1000, 400, -1, 2000));
    NPCTable.push_back(NPC(textureBee,    800, 300, -1, 2000));
    NPCTable.push_back(NPC(textureCloud,  100, 200,  1, -200));
    NPCTable.push_back(NPC(textureCloud,  250, 200,  1, -200));
    NPCTable.push_back(NPC(textureCloud,  450, 200,  1, -200));
    NPCTable.push_back(NPC(textureCloud,  650, 180,  1, -200));

    // ---- Repte 3: bosc d'arbres (struct Tree en un vector) ----
    std::vector<Tree> forest;
    for (int i = 0; i < NUM_TREES; i++) {
        int layer = i % 2;                       // alterna davant/darrere
        Texture& t = (layer == 0) ? textureTree2 : textureTree;
        Tree tree(t, layer);
        Vector2u ts = t.getSize();
        float scale = (layer == 0) ? 0.55f : 0.40f;   // els del davant mes grans
        tree.sprite.setScale({ scale, scale });
        tree.sprite.setOrigin({ ts.x / 2.0f, (float)ts.y });  // base centrada
        float x = 120.0f + i * (WINDOW_WIDTH - 240.0f) / (NUM_TREES - 1);
        float y = (layer == 0) ? 760.0f : 820.0f;
        tree.sprite.setPosition({ x, y });
        forest.push_back(tree);
    }

    // ---- HUD: textos ----
    Text messageText(font);
    messageText.setCharacterSize(75);
    messageText.setFillColor(Color::White);

    Text scoreText(font);
    scoreText.setCharacterSize(80);
    scoreText.setFillColor(Color::White);
    scoreText.setPosition({ 20, 20 });

    Text livesText(font);                 // Repte 4
    livesText.setCharacterSize(60);
    livesText.setFillColor(Color::White);
    livesText.setPosition({ 20, 120 });

    Text fpsText(font);                   // Repte 5
    fpsText.setCharacterSize(45);
    fpsText.setFillColor(Color::White);
    fpsText.setPosition({ 1600, 20 });

    Text modeText(font);                  // Repte 7: indicador de dificultat
    modeText.setCharacterSize(45);
    modeText.setFillColor(Color::Yellow);
    modeText.setPosition({ 1600, 90 });

    Text recordText(font);                // Repte 8: record
    recordText.setCharacterSize(45);
    recordText.setFillColor(Color::White);
    recordText.setPosition({ 1600, 160 });

    Text statsText(font);                 // estadistiques a la pausa
    statsText.setCharacterSize(55);
    statsText.setFillColor(Color::White);

    // Repte 4: panell fosc semitransparent
    RectangleShape overlay({ (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT });
    overlay.setFillColor(Color(0, 0, 0, 180));

    // ---- Barra de temps ----
    RectangleShape timeBar({ timeBarStartWidth, timeBarHeight });
    timeBar.setFillColor(Color::Red);
    timeBar.setPosition({ WINDOW_WIDTH / 2 - timeBarStartWidth / 2, 980 });

    // ---- Repte 6: branques en un sol vector d'estructures ----
    std::vector<Branch> branches;
    for (int i = 0; i < NUM_BRANCHES; i++) {
        Branch b(textureBranch);
        b.sprite.setOrigin({ 220, 20 });
        b.sprite.setPosition({ -2000, -2000 });
        branches.push_back(b);
    }

    // ---- Sons ----
    Sound chop(chopBuffer);
    Sound death(deathBuffer);
    Sound timeUp(timeBuffer);

    // ---- Variables de control ----
    Clock clock;
    bool  paused   = true;
    bool  gameOver = false;
    bool  started  = false;
    int   score    = 0;
    int   lives     = MAX_LIVES;
    GameMode mode  = GameMode::FACIL;              // Repte 7
    float timeRemaining = timeForMode(mode);
    float timeStart     = timeRemaining;
    Side  playerSide    = Side::LEFT;

    // Repte 5: control del refresc de l'FPS
    float fpsTimer = 0.0f;
    int   fpsValue = 0;

    // Repte 8: record persistent
    const std::string recordFile = "record.txt";
    int record = readRecord(recordFile);

    std::string message = "Prem Enter per començar!";

    // ============================================================
    //  BUCLE PRINCIPAL
    // ============================================================
    while (window.isOpen())
    {
        /***************  Entrada  ***************/
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<Event::Closed>())
                window.close();
            else if (const auto* keyPressed = event->getIf<Event::KeyPressed>())
            {
                if (keyPressed->scancode == Keyboard::Scancode::Escape)
                    window.close();

                // Repte 7: seleccio de mode amb 1 / 2 (nomes en pausa)
                if (paused && keyPressed->scancode == Keyboard::Scancode::Num1)
                    mode = GameMode::FACIL;
                if (paused && keyPressed->scancode == Keyboard::Scancode::Num2)
                    mode = GameMode::DIFICIL;

                // Enter: començar / reiniciar / pausar
                if (keyPressed->scancode == Keyboard::Scancode::Enter)
                {
                    if (paused)
                    {
                        // Repte 7: switch per decidir el temps inicial segons el mode
                        timeStart     = timeForMode(mode);
                        timeRemaining = timeStart;
                        score   = 0;
                        lives   = MAX_LIVES;
                        gameOver = false;
                        started  = true;
                        playerSide = Side::LEFT;
                        logActive  = false;
                        for (auto& b : branches) b.side = Side::NONE;   // Repte 6: iterador
                        spritePlayer.setPosition({ 675, 720 });
                        spriteRIP.setPosition({ 675, 2000 });
                        spriteAxe.setPosition({ 2000, 840 });
                        paused = false;
                    }
                    else
                        paused = true;   // pausa manual
                }

                // Tallar amb les fletxes
                if (!paused && keyPressed->scancode == Keyboard::Scancode::Left)
                {
                    playerSide = Side::LEFT;
                    score++;
                    // Repte 5: static_cast per no perdre decimals (divisio entera)
                    timeRemaining += (2.0f / static_cast<float>(score)) + 0.15f;
                    if (timeRemaining > timeStart) timeRemaining = timeStart;
                    spriteAxe.setPosition({ AXE_POSITION_LEFT, spriteAxe.getPosition().y });
                    spritePlayer.setPosition({ 675, 720 });
                    updateBranches(branches, score);
                    spriteLog.setPosition({ 810, 720 });
                    logSpeedX = 5000; logActive = true;
                    chop.play();
                    std::cout << "[TALL] Score=" << score
                              << "  FPS=" << fpsValue << std::endl;   // Repte 5
                }
                if (!paused && keyPressed->scancode == Keyboard::Scancode::Right)
                {
                    playerSide = Side::RIGHT;
                    score++;
                    timeRemaining += (2.0f / static_cast<float>(score)) + 0.15f;
                    if (timeRemaining > timeStart) timeRemaining = timeStart;
                    spriteAxe.setPosition({ AXE_POSITION_RIGHT, spriteAxe.getPosition().y });
                    spritePlayer.setPosition({ 1200, 720 });
                    updateBranches(branches, score);
                    spriteLog.setPosition({ 810, 720 });
                    logSpeedX = -5000; logActive = true;
                    chop.play();
                    std::cout << "[TALL] Score=" << score
                              << "  FPS=" << fpsValue << std::endl;
                }
            }
        }

        /***************  Mesura del temps  ***************/
        Time dt = clock.restart();

        /***************  Actualitzacio  ***************/
        if (!paused)
        {
            // Repte 9: assercions d'invariants (nomes en mode depuracio)
            assert(lives >= 0 && lives <= MAX_LIVES);
            assert(timeRemaining >= -1.0f);

            // Temps
            timeRemaining -= dt.asSeconds();
            if (timeRemaining <= 0.0f)
            {
                timeRemaining = 0.0f;
                paused = true; gameOver = true;
                message = "Temps esgotat!";
                timeUp.play();
                std::cout << "[FI] Temps esgotat. Score=" << score << std::endl;
                if (score > record) { record = score; writeRecord(recordFile, record); }
            }

            // Repte 9: continue per saltar els NPC inactius abans d'actualitzar
            for (size_t i = 0; i < NPCTable.size(); i++) {
                updateNPC(NPCTable[i], dt.asSeconds());
            }

            // Branques
            updateBranchSprites(branches);

            // Repte 9: cerca amb continue (saltem branques buides) i break (parem a la primera)
            bool squished = false;
            for (int i = NUM_BRANCHES - 1; i >= 0; i--) {
                if (branches[i].side == Side::NONE) continue;   // saltem les buides
                squished = (branches[i].side == playerSide && i == NUM_BRANCHES - 1);
                break;                                          // parem a la branca mes baixa
            }
            if (squished)
            {
                lives--;                         // Repte 4
                death.play();
                std::cout << "[VIDA] Perduda. Vides=" << lives
                          << "  Score=" << score << std::endl;
                if (lives <= 0)
                {
                    paused = true; gameOver = true;
                    message = "ESCLAFAT!";
                    spriteRIP.setPosition({ 525, 760 });
                    spritePlayer.setPosition({ 2000, 660 });
                    std::cout << "[FI] Game Over. Score=" << score << std::endl;
                    if (score > record) { record = score; writeRecord(recordFile, record); }
                }
                else
                {
                    // encara queden vides: netegem les branques i seguim
                    for (auto& b : branches) b.side = Side::NONE;
                }
            }

            // Tronc volador (Repte: moviment amb dt)
            if (logActive)
            {
                spriteLog.move({ logSpeedX * dt.asSeconds(), logSpeedY * dt.asSeconds() });
                if (spriteLog.getPosition().x < -100 || spriteLog.getPosition().x > 2000)
                    logActive = false;
            }

            // Repte 5: estimacio d'FPS, refrescada cada 0.25 s
            fpsTimer += dt.asSeconds();
            if (dt.asSeconds() > 0 && fpsTimer >= 0.25f)
            {
                fpsValue = static_cast<int>(1.0f / dt.asSeconds());
                fpsTimer = 0.0f;
            }
        }

        /***************  Textos del HUD  ***************/
        {
            std::stringstream ss; ss << "Score = " << score; scoreText.setString(ss.str());
        }
        {
            std::stringstream ss; ss << "Vides: " << lives; livesText.setString(ss.str());
        }
        {
            std::stringstream ss; ss << "FPS: " << fpsValue; fpsText.setString(ss.str());
        }
        {
            std::stringstream ss; ss << "Rècord: " << record; recordText.setString(ss.str());
        }
        modeText.setString(mode == GameMode::FACIL ? "Mode: Facil" : "Mode: Dificil");

        // Barra de temps centrada
        float barWidth = timeBarStartWidth * (timeRemaining / timeStart);
        if (barWidth < 0) barWidth = 0;
        timeBar.setSize({ barWidth, timeBarHeight });
        timeBar.setPosition({ WINDOW_WIDTH / 2 - barWidth / 2, 980 });

        /***************  Dibuix  ***************/
        window.clear();
        window.draw(spriteBackground);

        // Repte 3: bosc en dues capes (primer darrere, despres davant)
        for (const auto& tree : forest)
            if (tree.layer == 0) window.draw(tree.sprite);
        for (auto it = NPCTable.begin(); it != NPCTable.end(); ++it) {
            if (!it->active) continue;          // Repte 9: continue, saltem NPC inactius
            window.draw(it->sprite);            // Repte 6: recorregut amb iteradors
        }
        for (const auto& tree : forest)
            if (tree.layer == 1) window.draw(tree.sprite);

        window.draw(spriteTree);
        for (const auto& b : branches)          // Repte 6: dibuix amb for per interval
            window.draw(b.sprite);
        window.draw(spriteRIP);
        window.draw(spritePlayer);
        window.draw(spriteAxe);
        if (logActive) window.draw(spriteLog);

        // HUD
        window.draw(scoreText);
        window.draw(livesText);
        window.draw(fpsText);
        window.draw(modeText);
        window.draw(recordText);
        window.draw(timeBar);

        // Repte 4: panell semitransparent + missatge/estadistiques en pausa
        if (paused)
        {
            window.draw(overlay);

            std::string title = message;
            if (started) title = gameOver ? (message + "  -  GAME OVER") : "PAUSA";
            messageText.setString(title);
            FloatRect r = messageText.getLocalBounds();
            messageText.setOrigin({ r.position.x + r.size.x / 2.0f,
                                    r.position.y + r.size.y / 2.0f });
            messageText.setPosition({ WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 120.0f });
            window.draw(messageText);

            if (started)
            {
                std::stringstream ss;
                if (gameOver) ss << "Puntuacio final: " << score << "   Record: " << record;
                else          ss << "Score: " << score << "   Vides: " << lives;
                statsText.setString(ss.str());
            }
            else
            {
                statsText.setString(mode == GameMode::FACIL
                    ? "Mode: 1=Facil (actual)   2=Dificil"
                    : "Mode: 1=Facil   2=Dificil (actual)");
            }
            FloatRect sr = statsText.getLocalBounds();
            statsText.setOrigin({ sr.position.x + sr.size.x / 2.0f,
                                  sr.position.y + sr.size.y / 2.0f });
            statsText.setPosition({ WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f + 20.0f });
            window.draw(statsText);
        }

        window.display();
    }
    return 0;
}

// ============================================================
//  Repte 7: temps inicial segons el mode (switch)
// ============================================================
float timeForMode(GameMode mode) {
    switch (mode) {
    case GameMode::FACIL:   return 6.0f;
    case GameMode::DIFICIL: return 3.0f;
    default:                return 6.0f;
    }
}

// ============================================================
//  Repte 8: lectura i escriptura del record (try/catch)
// ============================================================
int readRecord(const std::string& path) {
    try {
        std::ifstream f(path);
        if (!f.is_open()) {                       // primera vegada: no existeix
            std::cout << "[RECORD] No hi ha fitxer de record, comencem a 0." << std::endl;
            return 0;
        }
        int r = 0;
        if (!(f >> r) || r < 0) {                 // fitxer buit o malmes
            std::cerr << "[RECORD] Fitxer no valid, es reinicia a 0." << std::endl;
            return 0;
        }
        std::cout << "[RECORD] Record carregat: " << r << std::endl;
        return r;
    }
    catch (const std::exception& e) {
        std::cerr << "[RECORD] Error llegint el record: " << e.what() << std::endl;
        return 0;
    }
}
void writeRecord(const std::string& path, int value) {
    try {
        std::ofstream f(path);
        if (!f.is_open()) throw std::runtime_error("no es pot obrir per escriure");
        f << value;
        std::cout << "[RECORD] Nou record desat: " << value << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "[RECORD] Error desant el record: " << e.what() << std::endl;
    }
}

// ============================================================
//  Repte 2: moviment dels NPC
// ============================================================
void updateNPC(NPC& npc, float dt) {
    if (!npc.active) {
        npc.speed = ((rand() % npc.maxSpeed) + npc.maxSpeed / 2) * npc.sentit;
        float height = (float)(rand() % npc.maxHeight);
        npc.sprite.setPosition({ npc.posicioInicialX, height });
        npc.active = true;
    }
    else {
        npc.sprite.setPosition({
            npc.sprite.getPosition().x + npc.speed * dt,
            npc.sprite.getPosition().y });
        if (npc.sprite.getPosition().x < -200 || npc.sprite.getPosition().x > 2000)
            npc.active = false;
    }
}

// ============================================================
//  Repte 6: desplaçar les branques (un sol vector d'estructures)
// ============================================================
void updateBranches(std::vector<Branch>& branches, int seed) {
    for (size_t j = branches.size() - 1; j > 0; j--)
        branches[j].side = branches[j - 1].side;

    srand((int)time(0) + seed);
    int r = rand() % 5;
    switch (r) {
    case 0:  branches[0].side = Side::LEFT;  break;
    case 1:  branches[0].side = Side::RIGHT; break;
    default: branches[0].side = Side::NONE;  break;
    }
}

// ============================================================
//  Repte 6: col·locar els sprites segons el costat
// ============================================================
void updateBranchSprites(std::vector<Branch>& branches) {
    int i = 0;
    for (auto& b : branches) {           // recorregut amb iteradors
        float height = i * 150.0f;
        if (b.side == Side::LEFT) {
            b.sprite.setPosition({ 610, height });
            b.sprite.setRotation(degrees(180));
        }
        else if (b.side == Side::RIGHT) {
            b.sprite.setPosition({ 1330, height });
            b.sprite.setRotation(degrees(0));
        }
        else {
            b.sprite.setPosition({ 3000, height });
        }
        i++;
    }
}
