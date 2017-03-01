#include <iostream>
#include "Page.h"
using namespace std;

Page::Page(){
	title = "";
	content = "";
}

void Page::displayPage(){
	cout << "title:" << getTitle() << endl;
	cout << "content:" << getContent() << endl;
}

string Page::getTitle(){
	return this->title;
}

string Page::getContent(){
	return this->content;
}

void Page::addTitle(string title){
 	this->title += title;
}

void Page::addContent(string additionalContent){
	this->content += additionalContent;
}

//extracts Page from line: <title>\t<content>
Page* Page::getPageFromLine(string& line){
	size_t found = line.find_first_of("\t");
    size_t len = found + 1;
    Page* page = new Page();
    page->addTitle(line.substr(0, len));
    page->addContent(line.substr(len));
    return page; 
}
