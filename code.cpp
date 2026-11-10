// =====================================================================
//  THE CONSTRUCTOR'S DILEMMA  -  A C++ Object Lifecycle Game
//  Project by : Hiten Kaushal (UID: 25BCD10144)
//  Class      : 25BCD-3A   |   Guided by: Abhinay (Assistant Professor)
// =====================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <limits>
#include <algorithm>

using namespace std;

// ---------- ★ TOGGLE ★ ----------
// 1 = full Lifecycle Trace after each answer  (recommended for demo)
// 0 = plain quiz mode (no trace shown)
#define SHOW_LIFECYCLE 1
// ------------------------------

// =====================================================================
//  CLASS 1 : Question  (Abstract Base Class)
// =====================================================================
class Question {
protected:
    string typeTag;
    vector<string> codeLines;
    string questionText;
    vector<string> options;
    int correctIndex;
    string explanation;
    vector<string> traceSteps;
    int points;

public:
    Question() : correctIndex(0), points(10) {}
    virtual ~Question() {}

    virtual string getBanner() const { return "QUESTION"; }
    virtual string getHint()   const { return ""; }

    void setTypeTag(const string& t)      { typeTag = t; }
    void addCodeLine(const string& l)     { codeLines.push_back(l); }
    void setQuestionText(const string& q) { questionText = q; }
    void addOption(const string& o)       { options.push_back(o); }
    void setCorrectIndex(int i)           { correctIndex = i; }
    void setExplanation(const string& e)  { explanation = e; }
    void addTraceStep(const string& s)    { traceSteps.push_back(s); }
    void setPoints(int p)                 { points = p; }

    int    getPoints()      const { return points; }
    int    getOptionCount() const { return (int)options.size(); }
    string getTypeTag()     const { return typeTag; }

    void display(int qNum) const {
        cout << "\n";
        cout << "==============================================================\n";
        cout << "  Q" << qNum << "  |  " << getBanner() << "  |  " << points << " pts\n";
        cout << "==============================================================\n";

        cout << "\n  Code:\n";
        cout << "  ------------------------------------------------------------\n";
        for (size_t i = 0; i < codeLines.size(); ++i)
            cout << "    " << codeLines[i] << "\n";
        cout << "  ------------------------------------------------------------\n";

        cout << "\n  " << questionText << "\n\n";

        for (size_t i = 0; i < options.size(); ++i)
            cout << "    " << (char)('A' + i) << ") " << options[i] << "\n";

        string hint = getHint();
        if (!hint.empty())
            cout << "\n  [Hint] " << hint << "\n";
    }

    bool isCorrect(int idx) const { return idx == correctIndex; }

    // ---------- The USP : Lifecycle Trace (toggle-able) ----------
    void showAnswerReveal() const {
        cout << "\n  Correct answer: " << (char)('A' + correctIndex)
             << ") " << options[correctIndex] << "\n";

#if SHOW_LIFECYCLE
        cout << "\n  -- Lifecycle Trace ---------------------------------------------\n";
        for (size_t i = 0; i < traceSteps.size(); ++i)
            cout << "  " << traceSteps[i] << "\n";
        cout << "  ---------------------------------------------------------------\n";
#endif

        if (!explanation.empty())
            cout << "\n  Why: " << explanation << "\n";
    }
};

// =====================================================================
//  DERIVED CLASSES
// =====================================================================
class CtorOrderQuestion : public Question {
public:
    string getBanner() const override { return "CONSTRUCTOR ORDER"; }
    string getHint()   const override {
        return "Base first -> then members (declaration order) -> then body.";
    }
};

class DtorOrderQuestion : public Question {
public:
    string getBanner() const override { return "DESTRUCTOR ORDER"; }
    string getHint()   const override {
        return "Destruction is the exact reverse of construction.";
    }
};

class VirtualDtorQuestion : public Question {
public:
    string getBanner() const override { return "VIRTUAL DESTRUCTOR"; }
    string getHint()   const override {
        return "Ask yourself: is ~Base() declared virtual? That changes everything.";
    }
};

class CopyCtorQuestion : public Question {
public:
    string getBanner() const override { return "COPY / ASSIGNMENT"; }
    string getHint()   const override {
        return "Initialization (Item b = a) uses copy ctor. Assignment (b = a) uses operator=.";
    }
};

Question* createQuestionByType(const string& type) {
    if (type == "CTOR")    return new CtorOrderQuestion();
    if (type == "DTOR")    return new DtorOrderQuestion();
    if (type == "VIRTUAL") return new VirtualDtorQuestion();
    if (type == "COPY")    return new CopyCtorQuestion();
    return new CtorOrderQuestion();
}

// =====================================================================
//  CLASS 2 : Player
// =====================================================================
class Player {
    string name;
    int score, lives, streak, bestStreak, correctCount, totalAnswered;

public:
    Player(const string& n = "Hiten")
        : name(n), score(0), lives(3), streak(0), bestStreak(0),
          correctCount(0), totalAnswered(0) {}

    void setName(const string& n) { name = n; }
    string getName()         const { return name; }
    int    getScore()        const { return score; }
    int    getLives()        const { return lives; }
    int    getStreak()       const { return streak; }
    int    getCorrectCount() const { return correctCount; }
    int    getTotalAnswered()const { return totalAnswered; }
    bool   isAlive()         const { return lives > 0; }

    int registerCorrect(int basePts) {
        correctCount++; totalAnswered++; streak++;
        if (streak > bestStreak) bestStreak = streak;

        int bonus = 0;
        if (streak >= 5)      bonus = 10;
        else if (streak == 3) bonus = 5;

        score += basePts + bonus;
        return bonus;
    }

    void registerWrong() {
        totalAnswered++;
        streak = 0;
        if (lives > 0) lives--;
    }

    void printStatus() const {
        cout << "\n  Player: " << name
             << "  |  Score: " << score
             << "  |  Lives: " << lives
             << "  |  Streak: " << streak << "\n";
    }
};

// =====================================================================
//  CLASS 3 : GameEngine
// =====================================================================
class GameEngine {
    vector<Question*> questions;
    Player player;
    string questionsFile, highScoreFile;
    map<string, pair<int,int> > conceptStats;

public:
    GameEngine(const string& qFile, const string& hFile)
        : player("Hiten"), questionsFile(qFile), highScoreFile(hFile) {}

    ~GameEngine() {
        for (size_t i = 0; i < questions.size(); ++i) delete questions[i];
        questions.clear();
    }

    void parseStream(istream& fin) {
        Question* current = NULL;
        string line;
        while (getline(fin, line)) {
            if (!line.empty() && line[line.size()-1] == '\r')
                line.erase(line.size()-1);
            if (line.empty() || line[0] == '#') continue;

            if (line.compare(0, 6, "@TYPE ") == 0) {
                if (current) questions.push_back(current);
                current = createQuestionByType(line.substr(6));
                current->setTypeTag(line.substr(6));
            }
            else if (!current) continue;
            else if (line.compare(0, 6,  "@CODE ")     == 0) current->addCodeLine(line.substr(6));
            else if (line.compare(0, 10, "@QUESTION ") == 0) current->setQuestionText(line.substr(10));
            else if (line.compare(0, 8,  "@OPTION ")   == 0) current->addOption(line.substr(8));
            else if (line.compare(0, 8,  "@ANSWER ")   == 0) current->setCorrectIndex(atoi(line.substr(8).c_str()) - 1);
            else if (line.compare(0, 9,  "@EXPLAIN ")  == 0) current->setExplanation(line.substr(9));
            else if (line.compare(0, 7,  "@TRACE ")    == 0) current->addTraceStep(line.substr(7));
            else if (line.compare(0, 8,  "@POINTS ")   == 0) current->setPoints(atoi(line.substr(8).c_str()));
            else if (line.compare(0, 4,  "@END")       == 0) {
                if (current) { questions.push_back(current); current = NULL; }
            }
        }
        if (current) questions.push_back(current);
    }

    void loadFallbackQuestions() {
        string data =
            "@TYPE CTOR\n@CODE class Demo {\n@CODE public:\n"
            "@CODE     Demo()  { cout << \"C\"; }\n"
            "@CODE     ~Demo() { cout << \"D\"; }\n@CODE };\n"
            "@CODE int main() { Demo a; Demo b; }\n"
            "@QUESTION What is the exact output?\n"
            "@OPTION C D C D\n@OPTION C C D D\n@OPTION D D C C\n@OPTION C D D C\n"
            "@ANSWER 2\n@EXPLAIN Objects are destroyed in the reverse order of creation.\n"
            "@TRACE [1] Demo a; -> ctor -> C\n"
            "@TRACE [2] Demo b; -> ctor -> C\n"
            "@TRACE [3] End of main: b destroyed -> D\n"
            "@TRACE [4] Then a destroyed -> D\n"
            "@POINTS 10\n@END\n"

            "@TYPE VIRTUAL\n"
            "@CODE class Base    { public: Base(){cout<<\"B\";} ~Base(){cout<<\"~B\";} };\n"
            "@CODE class Derived : public Base { public: Derived(){cout<<\"D\";} ~Derived(){cout<<\"~D\";} };\n"
            "@CODE int main() { Base* p = new Derived(); delete p; }\n"
            "@QUESTION Note: ~Base() is NOT virtual. What is the exact output?\n"
            "@OPTION B D ~D ~B\n@OPTION B D ~B\n@OPTION B D ~B ~D\n@OPTION D B ~B ~D\n"
            "@ANSWER 2\n@EXPLAIN Without virtual ~Base(), Derived's destructor is skipped -> leak.\n"
            "@TRACE [1] new Derived() -> B\n@TRACE [2] Derived ctor -> D\n"
            "@TRACE [3] delete p -> ~Base() only -> ~B\n"
            "@TRACE [4] ~Derived() never runs -> LEAK\n"
            "@POINTS 20\n@END\n"

            "@TYPE COPY\n"
            "@CODE class Item {\n@CODE public:\n"
            "@CODE     Item()            { cout << \"C\"; }\n"
            "@CODE     Item(const Item&) { cout << \"K\"; }\n"
            "@CODE     ~Item()           { cout << \"D\"; }\n@CODE };\n"
            "@CODE void take(Item x) {}\n@CODE int main() { Item a; take(a); }\n"
            "@QUESTION What is the exact output?\n"
            "@OPTION C D D\n@OPTION C K D D\n@OPTION C K K D D\n@OPTION C D K D\n"
            "@ANSWER 2\n@EXPLAIN Pass by value invokes the copy constructor.\n"
            "@TRACE [1] Item a -> C\n@TRACE [2] take(a) -> copy ctor -> K\n"
            "@TRACE [3] x destroyed at end of take -> D\n"
            "@TRACE [4] a destroyed at end of main -> D\n"
            "@POINTS 15\n@END\n"

            "@TYPE DTOR\n"
            "@CODE class A { public: A(){cout<<\"A\";} ~A(){cout<<\"~A\";} };\n"
            "@CODE class B { public: B(){cout<<\"B\";} ~B(){cout<<\"~B\";} };\n"
            "@CODE class Box { B b; A a;\n"
            "@CODE public: Box(){cout<<\"X\";} ~Box(){cout<<\"~X\";} };\n"
            "@CODE int main() { Box x; }\n"
            "@QUESTION What is the exact output?\n"
            "@OPTION A B X ~X ~B ~A\n@OPTION B A X ~X ~A ~B\n"
            "@OPTION B A X ~X ~B ~A\n@OPTION A B X ~X ~A ~B\n"
            "@ANSWER 2\n@EXPLAIN Members are destroyed in REVERSE declaration order.\n"
            "@TRACE [1] Member b (first) -> B\n@TRACE [2] Member a (second) -> A\n"
            "@TRACE [3] Box body -> X\n@TRACE [4] Box dtor -> ~X\n"
            "@TRACE [5] Member a destroyed -> ~A\n@TRACE [6] Member b destroyed -> ~B\n"
            "@POINTS 15\n@END\n";

        istringstream ss(data);
        parseStream(ss);
    }

    void loadQuestions() {
        ifstream fin(questionsFile.c_str());
        if (!fin.is_open()) {
            cout << "\n  [!] Could not open '" << questionsFile << "'.\n";
            cout << "  [!] Loading built-in fallback questions...\n\n";
            loadFallbackQuestions();
            return;
        }
        parseStream(fin);
        fin.close();
        if (questions.empty()) { loadFallbackQuestions(); return; }
        cout << "\n  [+] Loaded " << questions.size()
             << " questions from '" << questionsFile << "'.\n";
    }

    void shuffleQuestions() {
        int n = (int)questions.size();
        for (int i = n - 1; i > 0; --i)
            swap(questions[i], questions[rand() % (i + 1)]);
    }

    void showIntro() {
        cout << "\n";
        cout << "  ##############################################################\n";
        cout << "  #   T H E   C O N S T R U C T O R ' S   D I L E M M A        #\n";
        cout << "  #            A C++ Object Lifecycle Game                     #\n";
        cout << "  #                                                            #\n";
        cout << "  #   Project by : Hiten Kaushal  (25BCD10144)                #\n";
        cout << "  #   Class      : 25BCD-3A                                   #\n";
        cout << "  #   Guided by  : Mr.Abhinay Sood (Assistant Professor)                                #\n";
        cout << "  ##############################################################\n";
        cout << "\n  Predict the exact order of constructor & destructor calls.\n";
#if SHOW_LIFECYCLE
        cout << "  Each answer is followed by a step-by-step Lifecycle Trace.\n";
#endif
        cout << "\n  Rules: 3 lives. Correct = points (+ streak bonus). Wrong = lose life.\n";
        cout << "\n  Enter your name (or press ENTER for 'Hiten'): ";
        string name;
        getline(cin, name);
        if (name.empty()) name = "Hiten";
        player.setName(name);
    }

    void playQuestion(Question* q, int qNum) {
        q->display(qNum);
        player.printStatus();

        cout << "\n  Your answer (A/B/C/D, or 0 to quit): ";
        string input;
        while (true) {
            if (!getline(cin, input)) { input = "0"; break; }
            if (!input.empty()) break;
            cout << "  (Please enter A, B, C, or D): ";
        }

        if (input == "0" || input == "q" || input == "Q") {
            cout << "\n  Quitting early...\n";
            player.registerWrong();
            return;
        }

        int chosen = -1;
        char c = toupper(input[0]);
        if      (c >= 'A' && c <= 'D') chosen = c - 'A';
        else if (c >= '1' && c <= '4') chosen = c - '1';
        if (chosen < 0 || chosen >= q->getOptionCount()) {
            cout << "\n  Invalid choice. Counting as wrong.\n";
            chosen = -1;
        }

        string tag = q->getTypeTag();
        conceptStats[tag].second++;

        if (chosen >= 0 && q->isCorrect(chosen)) {
            int bonus = player.registerCorrect(q->getPoints());
            conceptStats[tag].first++;
            cout << "\n  [OK] Correct! +" << q->getPoints() << " pts";
            if (bonus > 0) cout << " (streak bonus +" << bonus << ")";
            cout << "\n";
        } else {
            player.registerWrong();
            cout << "\n  [X] Wrong! Lives left: " << player.getLives() << "\n";
        }

        q->showAnswerReveal();

        cout << "\n  Press ENTER to continue...";
        string dummy;
        getline(cin, dummy);
    }

    void showFinalReport() {
        cout << "\n\n==============================================================\n";
        cout << "                     GAME OVER\n";
        cout << "==============================================================\n";

        int total = player.getTotalAnswered();
        int correct = player.getCorrectCount();
        double acc = (total > 0) ? (100.0 * correct / total) : 0.0;

        cout << "  Player:             " << player.getName() << "\n";
        cout << "  Final Score:        " << player.getScore() << "\n";
        cout << "  Questions Answered: " << total << "\n";
        cout << "  Correct:            " << correct << "\n";
        cout << "  Accuracy:           " << fixed << setprecision(1) << acc << "%\n";
        cout << "  Lives Remaining:    " << player.getLives() << "\n";

        cout << "\n  -- Concept Breakdown -----------------------------------------\n";
        for (map<string, pair<int,int> >::iterator it = conceptStats.begin();
             it != conceptStats.end(); ++it) {
            cout << "    " << left << setw(10) << it->first
                 << "  " << it->second.first << " / " << it->second.second << "\n";
        }

        cout << "\n  Verdict: ";
        if      (acc >= 80) cout << "C++ Object Lifecycle Master!\n";
        else if (acc >= 60) cout << "Strong - review the ones you missed.\n";
        else                cout << "Keep practising - constructors are tricky.\n";
        cout << "==============================================================\n";
    }

    void saveHighScore() {
        int prev = 0;
        ifstream fin(highScoreFile.c_str());
        if (fin.is_open()) { fin >> prev; fin.close(); }

        if (player.getScore() > prev) {
            ofstream fout(highScoreFile.c_str());
            fout << player.getScore() << "\n" << player.getName() << "\n";
            fout.close();
            cout << "\n  * NEW HIGH SCORE saved: " << player.getScore()
                 << " pts (was " << prev << ")\n";
        } else {
            cout << "\n  High score remains: " << prev << " pts\n";
        }
    }

    void run() {
        srand((unsigned)time(0));
        loadQuestions();
        shuffleQuestions();
        showIntro();

        int qNum = 0;
        for (size_t i = 0; i < questions.size() && player.isAlive(); ++i)
            playQuestion(questions[i], ++qNum);

        showFinalReport();
        saveHighScore();
    }
};

// =====================================================================
//  MAIN
// =====================================================================
int main() {
    GameEngine engine("lifecycle_questions.txt", "highscore.txt");
    engine.run();
    cout << "\n  Thanks for playing - Hiten Kaushal (25BCD10144)\n\n";

#ifdef _WIN32
    system("pause");
#else
    cout << "  Press Enter to exit...\n";
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
#endif

    return 0;
}