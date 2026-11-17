// ============================================================
//  Farm Manager - an interactive C++ project showing OOP
//
//  Concepts shown (search for the tags like [ENCAPSULATION]):
//   1. Encapsulation        - private data + getters/setters
//   2. Constructors         - default, parameterized, copy, destructor
//   3. Inheritance          - Crop and Livestock extend FarmAsset
//   4. Abstraction          - FarmAsset is abstract (pure virtual)
//   5. Polymorphism         - virtual functions called through pointers
//   6. Function overloading - Farm::find() and Farm::totalValue()
//   7. Operator overloading - <<, ==, +=, -=, []
//   8. Pointers             - FarmAsset*, new / delete, this
//   9. Static member        - counts how many assets exist
//
//  The USER drives the program through a menu (cin / getline).
// ============================================================
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <stdexcept>
using namespace std;

// ------------------------------------------------------------
// Input helpers: keep asking until the user types something valid
// ------------------------------------------------------------
string readText(const string& prompt) {
    string s;
    while (true) {
        cout << prompt;
        if (!getline(cin, s)) throw runtime_error("Input closed.");
        if (!s.empty()) return s;
        cout << "  Please type something.\n";
    }
}

int readInt(const string& prompt, int minValue = 0) {
    while (true) {
        string s = readText(prompt);
        try {
            size_t used = 0;
            int value = stoi(s, &used);
            if (used == s.size() && value >= minValue) return value;
        } catch (...) {}
        cout << "  Enter a whole number (minimum " << minValue << ").\n";
    }
}

double readDouble(const string& prompt) {
    while (true) {
        string s = readText(prompt);
        try {
            size_t used = 0;
            double value = stod(s, &used);
            if (used == s.size() && value >= 0) return value;
        } catch (...) {}
        cout << "  Enter a number that is 0 or more.\n";
    }
}

// ------------------------------------------------------------
// BASE CLASS (abstract): FarmAsset
// ------------------------------------------------------------
class FarmAsset {
private:                         // [ENCAPSULATION] outside code cannot touch these
    string name;
    int quantity;
    double unitPrice;
    static int count;            // [STATIC] shared by ALL assets

public:
    // [CONSTRUCTORS]
    FarmAsset() : name("Unknown"), quantity(0), unitPrice(0.0) { count++; }

    FarmAsset(const string& n, int q, double p)
        : name(n), quantity(0), unitPrice(0.0) {
        if (q >= 0) quantity = q;      // validate before storing
        setUnitPrice(p);
        count++;
    }

    FarmAsset(const FarmAsset& other)  // copy constructor
        : name(other.name), quantity(other.quantity), unitPrice(other.unitPrice) {
        count++;
    }

    virtual ~FarmAsset() { count--; }  // virtual destructor (important!)

    // [ENCAPSULATION] getters
    string getName() const { return name; }
    int getQuantity() const { return quantity; }
    double getUnitPrice() const { return unitPrice; }
    static int getCount() { return count; }

    // [ENCAPSULATION] setter with validation
    void setUnitPrice(double p) {
        if (p < 0) { cout << "  [Error] Price cannot be negative.\n"; return; }
        unitPrice = p;
    }

    // [ABSTRACTION] every child MUST provide these
    virtual string getType() const = 0;
    virtual string unit() const = 0;
    virtual string extra() const = 0;

    // [POLYMORPHISM] children may change how value is worked out
    virtual double estimatedValue() const { return quantity * unitPrice; }

    // [OPERATOR OVERLOADING] asset += 5;   (harvested / born)
    FarmAsset& operator+=(int amount) {
        if (amount <= 0) cout << "  [Error] Amount must be positive.\n";
        else quantity += amount;
        return *this;            // [POINTER] 'this' is the current object
    }

    // [OPERATOR OVERLOADING] asset -= 5;   (sold / lost)
    FarmAsset& operator-=(int amount) {
        if (amount <= 0) cout << "  [Error] Amount must be positive.\n";
        else if (amount > quantity) cout << "  [Error] You only have "
                                          << quantity << " " << unit() << ".\n";
        else quantity -= amount;
        return *this;
    }

    // [OPERATOR OVERLOADING] same name AND same type = same asset
    bool operator==(const FarmAsset& other) const {
        return name == other.name && getType() == other.getType();
    }

    // [OPERATOR OVERLOADING] cout << asset;
    friend ostream& operator<<(ostream& os, const FarmAsset& a) {
        os << left << setw(14) << a.name
           << setw(11) << a.getType()
           << setw(6) << a.quantity << setw(6) << a.unit()
           << "@ KES " << setw(11) << fixed << setprecision(2) << a.unitPrice
           << "Value: KES " << setw(10) << a.estimatedValue()
           << a.extra();
        return os;
    }
};

int FarmAsset::count = 0;        // static members are defined outside the class

// ------------------------------------------------------------
// DERIVED CLASS 1: Crop (we assume 5% is lost in storage)
// ------------------------------------------------------------
class Crop : public FarmAsset {
private:
    string season;

public:
    Crop() : FarmAsset(), season("Unknown") {}
    Crop(const string& n, int kg, double pricePerKg, const string& s)
        : FarmAsset(n, kg, pricePerKg), season(s) {}   // parent constructor first

    string getType() const override { return "Crop"; }
    string unit() const override { return "kg"; }
    string extra() const override { return "Season: " + season; }

    // [POLYMORPHISM] crops lose 5% value in storage
    double estimatedValue() const override { return FarmAsset::estimatedValue() * 0.95; }
};

// ------------------------------------------------------------
// DERIVED CLASS 2: Livestock
// ------------------------------------------------------------
class Livestock : public FarmAsset {
private:
    string breed;

public:
    Livestock() : FarmAsset(), breed("Unknown") {}
    Livestock(const string& n, int heads, double pricePerHead, const string& b)
        : FarmAsset(n, heads, pricePerHead), breed(b) {}

    string getType() const override { return "Livestock"; }
    string unit() const override { return "head"; }
    string extra() const override { return "Breed: " + breed; }
};

// ------------------------------------------------------------
// Farm: OWNS the assets through pointers
// ------------------------------------------------------------
class Farm {
private:
    vector<FarmAsset*> assets;

public:
    Farm() {}

    // The farm owns raw pointers, so copying it would be dangerous.
    // We switch copying off on purpose.
    Farm(const Farm&) = delete;
    Farm& operator=(const Farm&) = delete;

    ~Farm() {                              // [POINTERS] free what we created with 'new'
        for (FarmAsset* a : assets) delete a;
    }

    size_t size() const { return assets.size(); }

    // [OPERATOR OVERLOADING] farm += new Crop(...);
    Farm& operator+=(FarmAsset* newAsset) {
        for (FarmAsset* a : assets) {
            if (*a == *newAsset) {         // uses FarmAsset::operator==
                cout << "  [Error] " << newAsset->getName()
                     << " already exists. Use option 3 to add stock.\n";
                delete newAsset;           // we own it, so we must free it
                return *this;
            }
        }
        assets.push_back(newAsset);
        cout << "  Added: " << newAsset->getName() << "\n";
        return *this;
    }

    // [OPERATOR OVERLOADING] farm[0] gives the first asset
    FarmAsset* operator[](size_t index) const {
        if (index >= assets.size()) return nullptr;
        return assets[index];
    }

    // [FUNCTION OVERLOADING] find by number...
    FarmAsset* find(int number) const {
        if (number < 1) return nullptr;
        return (*this)[number - 1];
    }
    // ...or find by name (same function name, different parameter)
    FarmAsset* find(const string& name) const {
        for (FarmAsset* a : assets)
            if (a->getName() == name) return a;
        return nullptr;
    }

    // [FUNCTION OVERLOADING] total of everything...
    double totalValue() const {
        double sum = 0;
        for (FarmAsset* a : assets)
            sum += a->estimatedValue();    // [POLYMORPHISM] correct version chosen at runtime
        return sum;
    }
    // ...or total of one type only
    double totalValue(const string& type) const {
        double sum = 0;
        for (FarmAsset* a : assets)
            if (a->getType() == type) sum += a->estimatedValue();
        return sum;
    }

    void list() const {
        if (assets.empty()) { cout << "  (nothing recorded yet)\n"; return; }
        int n = 1;
        for (FarmAsset* a : assets)
            cout << "  " << n++ << ". " << *a << "\n";
    }
};

// ------------------------------------------------------------
// Menu  
// ------------------------------------------------------------
void showMenu() {
    cout << "\n=========== FARM MANAGER ===========\n"
         << " 1. Add a crop\n"
         << " 2. Add livestock\n"
         << " 3. Add stock   (harvest / newborn)\n"
         << " 4. Remove stock (sold / lost)\n"
         << " 5. View all records\n"
         << " 6. Total farm value\n"
         << " 7. Value of crops or livestock only\n"
         << " 8. Search by name\n"
         << " 0. Exit\n"
         << "====================================\n";
}

FarmAsset* pickByNumber(const Farm& farm) {
    if (farm.size() == 0) { cout << "  Nothing recorded yet.\n"; return nullptr; }
    farm.list();
    int number = readInt("Enter the number: ", 1);
    FarmAsset* a = farm.find(number);              // overload 1: by number
    if (a == nullptr) cout << "  No record with that number.\n";
    return a;
}

void runMenu(Farm& farm) {
    cout << "Welcome to Farm Manager (type the number of your choice).\n";
    while (true) {
        showMenu();
        int choice = readInt("Choice: ");

        if (choice == 0) {
            cout << "Goodbye! Total records this session: "
                 << FarmAsset::getCount() << "\n";
            return;
        }
        else if (choice == 1) {
            string name   = readText("Crop name: ");
            int kg        = readInt("Quantity (kg): ");
            double price  = readDouble("Price per kg (KES): ");
            string season = readText("Season (e.g. Long rains): ");
            farm += new Crop(name, kg, price, season);
        }
        else if (choice == 2) {
            string name   = readText("Animal name (e.g. Cow): ");
            int heads     = readInt("Number of animals: ");
            double price  = readDouble("Price per animal (KES): ");
            string breed  = readText("Breed: ");
            farm += new Livestock(name, heads, price, breed);
        }
        else if (choice == 3) {
            FarmAsset* a = pickByNumber(farm);
            if (a) {
                int amount = readInt("How many " + a->unit() + " to add? ", 1);
                *a += amount;                        // operator+=
                cout << "  Now: " << *a << "\n";
            }
        }
        else if (choice == 4) {
            FarmAsset* a = pickByNumber(farm);
            if (a) {
                int amount = readInt("How many " + a->unit() + " to remove? ", 1);
                *a -= amount;                        // operator-=
                cout << "  Now: " << *a << "\n";
            }
        }
        else if (choice == 5) {
            cout << "\n--- All records ---\n";
            farm.list();
        }
        else if (choice == 6) {
            cout << fixed << setprecision(2)
                 << "  Total farm value: KES " << farm.totalValue() << "\n";   // overload 1
        }
        else if (choice == 7) {
            int t = readInt("1 = Crop, 2 = Livestock: ", 1);
            string type = (t == 2) ? "Livestock" : "Crop";
            cout << fixed << setprecision(2)
                 << "  Total " << type << " value: KES " << farm.totalValue(type) << "\n"; // overload 2
        }
        else if (choice == 8) {
            string name = readText("Name to search: ");
            FarmAsset* a = farm.find(name);          // overload 2: by name
            if (a) cout << "  Found: " << *a << "\n";
            else   cout << "  Not found.\n";
        }
        else {
            cout << "  Please choose a number from the menu.\n";
        }
    }
}

int main() {
    Farm farm;                    // destructor runs automatically at the end
    try {
        runMenu(farm);
    } catch (const exception& e) {
        cout << "\n" << e.what() << " Closing the program.\n";
    }
    return 0;
}