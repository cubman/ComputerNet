#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp> 
#include <sstream> 
#include <vector>
#include <boost/algorithm/string/replace.hpp>

using namespace std;

string start_html = "Content-Type: text/html; charset=utf-8\n\n";
string end_html = "</body></html>";

string f_name = "editText.txt";

void page_not_found() {
	cout << "<html> <head> <title> 404 error </title> </head> <body> <h1>" << 
        "Page not found!" << "</h1>";
}

string urlDecode(string &SRC) {
    string ret;
    char ch;
    int i, ii;
    for (i=0; i<SRC.length(); i++) {
        if (int(SRC[i])==37) {
            sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
            ch=static_cast<char>(ii);
            ret+=ch;
            i=i+2;
        } else {
            ret+=SRC[i];
        }
    }
    return (ret);
}

void edit_page() {
	ifstream ifs(f_name);
	string s;

    cout << "<html> <head> <title> Welcome </title> </head> <body>";
    if (ifs) 
		while(std::getline(ifs, s))
      		cout << "<p>" << s << "</p>";
    
	cout << "<form  method=\"GET\" action=\"index.cgi\">"
		 << "<INPUT type=\"hidden\" value=\"edit\" name= \"type\">" 
		 << "<INPUT TYPE=submit VALUE=\"Edit\">"
		 << "</FORM>";
		 ifs.close();
}

void save_page() {
	ifstream ifs(f_name);
	string s;

    cout << "<html> <head> <title> Welcome </title> </head> <body>";
    cout << "<form method=\"post\" action=\"index.cgi\">"
    	 << "<INPUT type=\"hidden\" value=\"save\" name= \"type\">" 
    	 << "<textarea rows=\"10\" cols=\"45\" name=\"text\">";
    if (ifs) 
		while(std::getline(ifs, s))
      		cout << s << endl;
      	
    cout << "</textarea>"
		 << "<INPUT TYPE=submit VALUE=\"Save\">"
		 << "</FORM>";
		 ifs.close();
}

void finish_page(string text) {
	text = urlDecode(text);
	/*boost::replace_all(text, "+", " ");
	boost::replace_all(text, "%2B", "+");
	boost::replace_all(text, "%0D%0A", "\n");
	boost::replace_all(text, "%21", "!");
	boost::replace_all(text, "%40", "@");
	boost::replace_all(text, "%23", "#");
	boost::replace_all(text, "%24", "$");
	boost::replace_all(text, "%25", "%");
	boost::replace_all(text, "%5E", "^");
	boost::replace_all(text, "%26", "&");
	boost::replace_all(text, "%28", "(");
	boost::replace_all(text, "%29", ")");
	boost::replace_all(text, "%3F", "?");
	boost::replace_all(text, "%2C", ",");
	boost::replace_all(text, "%22", "\"");
	boost::replace_all(text, "%27", "'");
	boost::replace_all(text, "%3B", ";");
	boost::replace_all(text, "%3A", ":");
	boost::replace_all(text, "%3C", "<");
	boost::replace_all(text, "%3E", ">");
	boost::replace_all(text, "%5C", "\\");
	boost::replace_all(text, "%7B", "{");
	boost::replace_all(text, "%7D", "}");
	boost::replace_all(text, "%5B", "[");
	boost::replace_all(text, "%5D", "]");
	boost::replace_all(text, "%3D", "=");*/


	ofstream ofs(f_name);
	ofs << text;  
	ofs.close();
	if (access( f_name.c_str(), F_OK ) != -1)
		edit_page();
	else
		page_not_found();
	
}

constexpr unsigned int str2int(const char* str, int h = 0)
{
    return !str[h] ? 5381 : (str2int(str, h+1) * 33) ^ str[h];
}

int main()
{
    char * queryString = getenv("QUERY_STRING");
    char *length = getenv("CONTENT_LENGTH");
	
    vector<string> strs;
    
    cout << start_html;
    if (length) {
    	ostringstream out;
    	out << cin.rdbuf();
        string res = out.str();
        
        boost::split( strs, res, boost::is_any_of( "&=" ));
        if (strs.size() == 4 && strs[0] == "type" 
				        	 && strs[2] == "text")
        	switch(str2int(strs[1].c_str())) {
    			case str2int("save") :
    				finish_page(strs[3]);
    				break;
    			default :
    				page_not_found();
    				break;
    			}
    } else
    if (queryString) {
		boost::split( strs, queryString, boost::is_any_of( "=" ));
    	if (strs.size() == 2 && strs[0] == "type")
    		switch(str2int(strs[1].c_str())) {
    			case str2int("edit") :
    				save_page();
    				break;
    			default :	
    				page_not_found();
    				break;
    		}
    		else 
    		edit_page();
    	} else 
    		edit_page();

    return 0;
}