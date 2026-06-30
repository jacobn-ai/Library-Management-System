/*
    Library Management System
    ----------------------------
    A console-based application in C++ demonstrating object-oriented
    programming and persistent file handling for managing books,
    members, and borrowing records.

    Files used:
      books.dat      -> stores Book records
      members.dat    -> stores Member records

    Features:
      - Add Book
      - Display All Books
      - Search Book by Title or Author
      - Register Member
      - Display All Members
      - Issue Book
      - Return Book
      - View Currently Issued Books

    Compile:  g++ -o library library_management_system.cpp
    Run:      ./library
*/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <limits>
#include <algorithm>

using namespace std;

const char* BOOKS_FILE   = "books.dat";
const char* MEMBERS_FILE = "members.dat";

// =====================================================
//                     Book Class
// =====================================================
class Book {
private:
    int  bookId;
    char title[60];
    char author[40];
    char isbn[20];
    int  totalCopies;
    int  availableCopies;

public:
    void setData(int id, const char* t, const char* a, const char* i, int copies) {
        bookId = id;
        strncpy(title, t, sizeof(title) - 1);  title[sizeof(title) - 1] = '\0';
        strncpy(author, a, sizeof(author) - 1); author[sizeof(author) - 1] = '\0';
        strncpy(isbn, i, sizeof(isbn) - 1);     isbn[sizeof(isbn) - 1] = '\0';
        totalCopies = copies;
        availableCopies = copies;
    }

    int    getId()        const { return bookId; }
    string getTitle()     const { return title; }
    string getAuthor()    const { return author; }
    string getIsbn()      const { return isbn; }
    int    getTotal()     const { return totalCopies; }
    int    getAvailable() const { return availableCopies; }

    bool issueCopy() {
        if (availableCopies <= 0) return false;
        availableCopies--;
        return true;
    }

    bool returnCopy() {
        if (availableCopies >= totalCopies) return false;
        availableCopies++;
        return true;
    }

    void display() const {
        cout << left
             << setw(6)  << bookId
             << setw(28) << title
             << setw(20) << author
             << setw(8)  << totalCopies
             << setw(10) << availableCopies
             << "\n";
    }

    // Case-insensitive substring match helper
    static string toLower(const string& s) {
        string r = s;
        transform(r.begin(), r.end(), r.begin(), ::tolower);
        return r;
    }

    bool matchesTitle(const string& query) const {
        return toLower(title).find(toLower(query)) != string::npos;
    }

    bool matchesAuthor(const string& query) const {
        return toLower(author).find(toLower(query)) != string::npos;
    }
};

// =====================================================
//                    Member Class
// =====================================================
class Member {
private:
    int  memberId;
    char name[50];
    char phone[15];

public:
    void setData(int id, const char* n, const char* p) {
        memberId = id;
        strncpy(name, n, sizeof(name) - 1);   name[sizeof(name) - 1] = '\0';
        strncpy(phone, p, sizeof(phone) - 1); phone[sizeof(phone) - 1] = '\0';
    }

    int    getId()    const { return memberId; }
    string getName()  const { return name; }
    string getPhone() const { return phone; }

    void display() const {
        cout << left
             << setw(8)  << memberId
             << setw(25) << name
             << setw(15) << phone
             << "\n";
    }
};

// =====================================================
//                  Issue Record Struct
// =====================================================
// Stored in a separate binary file "issues.dat"
const char* ISSUES_FILE = "issues.dat";

struct IssueRecord {
    int  bookId;
    int  memberId;
    char issueDate[12];
    bool returned;
};

// =====================================================
//                  Utility Functions
// =====================================================
void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int readInt(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (cin.fail()) {
            clearInputBuffer();
            cout << "Invalid input. Please enter a number.\n";
        } else {
            clearInputBuffer();
            return value;
        }
    }
}

void readLine(const string& prompt, char* buffer, size_t size) {
    cout << prompt;
    cin.getline(buffer, size);
}

#include <ctime>
string currentDate() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buf[12];
    snprintf(buf, sizeof(buf), "%02d-%02d-%04d",
             ltm->tm_mday, 1 + ltm->tm_mon, 1900 + ltm->tm_year);
    return string(buf);
}

// =====================================================
//                  Book Operations
// =====================================================
int getNextBookId() {
    ifstream file(BOOKS_FILE, ios::binary);
    int maxId = 100;
    Book b;
    while (file.read(reinterpret_cast<char*>(&b), sizeof(Book))) {
        if (b.getId() > maxId) maxId = b.getId();
    }
    file.close();
    return maxId + 1;
}

void addBook() {
    char title[60], author[40], isbn[20];
    int id = getNextBookId();

    cout << "\n--- Add New Book ---\n";
    readLine("Enter Title: ", title, sizeof(title));
    readLine("Enter Author: ", author, sizeof(author));
    readLine("Enter ISBN: ", isbn, sizeof(isbn));
    int copies = readInt("Enter Number of Copies: ");
    if (copies < 1) copies = 1;

    Book b;
    b.setData(id, title, author, isbn, copies);

    ofstream file(BOOKS_FILE, ios::binary | ios::app);
    if (!file) {
        cout << "Error: Could not open books file.\n";
        return;
    }
    file.write(reinterpret_cast<char*>(&b), sizeof(Book));
    file.close();

    cout << "Book added successfully! Book ID: " << id << "\n";
}

void printBookHeader() {
    cout << "\n" << left
         << setw(6)  << "ID"
         << setw(28) << "Title"
         << setw(20) << "Author"
         << setw(8)  << "Total"
         << setw(10) << "Available" << "\n";
    cout << string(72, '-') << "\n";
}

void displayAllBooks() {
    ifstream file(BOOKS_FILE, ios::binary);
    if (!file) {
        cout << "No books found.\n";
        return;
    }
    Book b;
    bool found = false;
    printBookHeader();
    while (file.read(reinterpret_cast<char*>(&b), sizeof(Book))) {
        b.display();
        found = true;
    }
    file.close();
    if (!found) cout << "No books to display.\n";
}

void searchBooks() {
    cout << "\nSearch by: 1-Title  2-Author\n";
    int choice = readInt("Enter choice: ");

    char query[60];
    readLine("Enter search text: ", query, sizeof(query));
    string q(query);

    ifstream file(BOOKS_FILE, ios::binary);
    if (!file) {
        cout << "No books found.\n";
        return;
    }

    Book b;
    bool found = false;
    printBookHeader();
    while (file.read(reinterpret_cast<char*>(&b), sizeof(Book))) {
        bool match = (choice == 2) ? b.matchesAuthor(q) : b.matchesTitle(q);
        if (match) {
            b.display();
            found = true;
        }
    }
    file.close();

    if (!found) cout << "No matching books found.\n";
}

bool findBookById(int id, Book& result, streampos& pos) {
    ifstream file(BOOKS_FILE, ios::binary);
    if (!file) return false;
    Book b;
    while (file.read(reinterpret_cast<char*>(&b), sizeof(Book))) {
        if (b.getId() == id) {
            result = b;
            pos = file.tellg();
            pos -= static_cast<streamoff>(sizeof(Book));
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

void rewriteBook(const Book& b, streampos pos) {
    fstream file(BOOKS_FILE, ios::binary | ios::in | ios::out);
    file.seekp(pos);
    file.write(reinterpret_cast<const char*>(&b), sizeof(Book));
    file.close();
}

// =====================================================
//                Member Operations
// =====================================================
int getNextMemberId() {
    ifstream file(MEMBERS_FILE, ios::binary);
    int maxId = 1000;
    Member m;
    while (file.read(reinterpret_cast<char*>(&m), sizeof(Member))) {
        if (m.getId() > maxId) maxId = m.getId();
    }
    file.close();
    return maxId + 1;
}

void registerMember() {
    char name[50], phone[15];
    int id = getNextMemberId();

    cout << "\n--- Register New Member ---\n";
    readLine("Enter Name: ", name, sizeof(name));
    readLine("Enter Phone Number: ", phone, sizeof(phone));

    Member m;
    m.setData(id, name, phone);

    ofstream file(MEMBERS_FILE, ios::binary | ios::app);
    if (!file) {
        cout << "Error: Could not open members file.\n";
        return;
    }
    file.write(reinterpret_cast<char*>(&m), sizeof(Member));
    file.close();

    cout << "Member registered successfully! Member ID: " << id << "\n";
}

void displayAllMembers() {
    ifstream file(MEMBERS_FILE, ios::binary);
    if (!file) {
        cout << "No members found.\n";
        return;
    }
    Member m;
    bool found = false;

    cout << "\n" << left
         << setw(8)  << "ID"
         << setw(25) << "Name"
         << setw(15) << "Phone" << "\n";
    cout << string(48, '-') << "\n";

    while (file.read(reinterpret_cast<char*>(&m), sizeof(Member))) {
        m.display();
        found = true;
    }
    file.close();
    if (!found) cout << "No members to display.\n";
}

bool memberExists(int id) {
    ifstream file(MEMBERS_FILE, ios::binary);
    if (!file) return false;
    Member m;
    while (file.read(reinterpret_cast<char*>(&m), sizeof(Member))) {
        if (m.getId() == id) { file.close(); return true; }
    }
    file.close();
    return false;
}

// =====================================================
//              Issue / Return Operations
// =====================================================
void issueBook() {
    cout << "\n--- Issue Book ---\n";
    int bookId = readInt("Enter Book ID: ");
    int memberId = readInt("Enter Member ID: ");

    if (!memberExists(memberId)) {
        cout << "Member not found. Please register the member first.\n";
        return;
    }

    Book b;
    streampos pos;
    if (!findBookById(bookId, b, pos)) {
        cout << "Book not found.\n";
        return;
    }

    if (!b.issueCopy()) {
        cout << "No available copies of this book right now.\n";
        return;
    }
    rewriteBook(b, pos);

    IssueRecord rec;
    rec.bookId = bookId;
    rec.memberId = memberId;
    strncpy(rec.issueDate, currentDate().c_str(), sizeof(rec.issueDate) - 1);
    rec.issueDate[sizeof(rec.issueDate) - 1] = '\0';
    rec.returned = false;

    ofstream file(ISSUES_FILE, ios::binary | ios::app);
    file.write(reinterpret_cast<char*>(&rec), sizeof(IssueRecord));
    file.close();

    cout << "Book \"" << b.getTitle() << "\" issued to member ID "
         << memberId << " successfully.\n";
}

void returnBook() {
    cout << "\n--- Return Book ---\n";
    int bookId = readInt("Enter Book ID: ");
    int memberId = readInt("Enter Member ID: ");

    fstream file(ISSUES_FILE, ios::binary | ios::in | ios::out);
    if (!file) {
        cout << "No issue records found.\n";
        return;
    }

    IssueRecord rec;
    bool found = false;

    while (file.read(reinterpret_cast<char*>(&rec), sizeof(IssueRecord))) {
        if (rec.bookId == bookId && rec.memberId == memberId && !rec.returned) {
            rec.returned = true;
            streampos pos = file.tellg();
            pos -= static_cast<streamoff>(sizeof(IssueRecord));
            file.seekp(pos);
            file.write(reinterpret_cast<char*>(&rec), sizeof(IssueRecord));
            found = true;
            break;
        }
    }
    file.close();

    if (!found) {
        cout << "No active issue record found for this Book ID / Member ID pair.\n";
        return;
    }

    Book b;
    streampos pos;
    if (findBookById(bookId, b, pos)) {
        b.returnCopy();
        rewriteBook(b, pos);
    }

    cout << "Book returned successfully.\n";
}

void viewIssuedBooks() {
    ifstream file(ISSUES_FILE, ios::binary);
    if (!file) {
        cout << "No issue records found.\n";
        return;
    }

    IssueRecord rec;
    bool found = false;

    cout << "\n" << left
         << setw(10) << "BookID"
         << setw(12) << "MemberID"
         << setw(14) << "IssueDate"
         << setw(10) << "Status" << "\n";
    cout << string(46, '-') << "\n";

    while (file.read(reinterpret_cast<char*>(&rec), sizeof(IssueRecord))) {
        if (!rec.returned) {
            cout << left
                 << setw(10) << rec.bookId
                 << setw(12) << rec.memberId
                 << setw(14) << rec.issueDate
                 << setw(10) << "Issued" << "\n";
            found = true;
        }
    }
    file.close();

    if (!found) cout << "No books are currently issued.\n";
}

// =====================================================
//                       Menu
// =====================================================
void showMenu() {
    cout << "\n=========================================\n";
    cout << "         LIBRARY MANAGEMENT SYSTEM\n";
    cout << "=========================================\n";
    cout << "1. Add Book\n";
    cout << "2. Display All Books\n";
    cout << "3. Search Book (Title/Author)\n";
    cout << "4. Register Member\n";
    cout << "5. Display All Members\n";
    cout << "6. Issue Book\n";
    cout << "7. Return Book\n";
    cout << "8. View Currently Issued Books\n";
    cout << "9. Exit\n";
    cout << "=========================================\n";
}

int main() {
    int choice;

    do {
        showMenu();
        choice = readInt("Enter your choice (1-9): ");

        switch (choice) {
            case 1: addBook();           break;
            case 2: displayAllBooks();   break;
            case 3: searchBooks();       break;
            case 4: registerMember();    break;
            case 5: displayAllMembers(); break;
            case 6: issueBook();         break;
            case 7: returnBook();        break;
            case 8: viewIssuedBooks();   break;
            case 9: cout << "Exiting Library Management System. Goodbye!\n"; break;
            default: cout << "Invalid choice. Please select between 1 and 9.\n";
        }

    } while (choice != 9);

    return 0;
}
