#include <libxml++/libxml++.h>

using namespace std;

class Page{
private:
	string title;
	string content;
public:
	Page();
	void displayPage();
	string getTitle();
	string getContent();
	void addTitle(string title);
	void setContent(string content);
	void addContent(string additionalContent);
	static Page* getPageFromLine(string& line);
};