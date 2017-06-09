#include <fstream>
#include <iostream>
#include <string>
#include <dirent.h> 
#include <map>
#include <boost/filesystem.hpp>
#include <dirent.h>

using namespace std;
using namespace boost::filesystem;

string cur_dir = "./";
map<string, int> mapM;
string end_html = "</body> </html>\n";

void get_files(path dir) {
    int cnt = 1;

    cout << "<table>\n";
    for (directory_iterator itr(dir), end; itr != end; ++itr)
        {
            if (cnt == 1) {
                cout << "<tr>\n";
            }

            // If it's not a directory, list it. If you want to list directories too, just remove this check.
            if (is_regular_file(itr->path())) {
                // assign current file name to current_file and echo it out to the console.
              //ftp://stud.mmcs.sfedu.ru/public_html/cgi/index.cgi
                cout << "<td>\n";
                string current_file = itr->path().string();
                current_file = current_file.substr(cur_dir.size());
                cout <<"<a href=\'http://stud.mmcs.sfedu.ru/~GeorgGreat/cgi/" <<current_file << "\'>"
                	<< "<img src=\'" << current_file <<  "\' style= \'width:" << mapM["iconWidth"] << "px; height:" << mapM["iconHeight"] << "px;" << "\'> "
                	 <<"</a>";
                cout << "</td>\n";
            }
            if (!(cnt % mapM["columnCount"])) {
                cout << "</tr>\n";
                cnt = 0;
            }
            ++cnt;
        }

        if (cnt != 1)
            cout << "</tr>\n";
        
        cout << "</table>\n";
}

void enter_page() {
    cout << "Content-Type: text/html; charset=utf-8\n\n"
		 << "<html> <head> <title> main content</title> </head> <body>\n" 
  	     << "<h1> Gallery </h1>\n"
    	 << "<p>let's start ... </p>\n";
}

void page_not_found() {
    cout << "Content-Type: text/html; charset=utf-8\n\n"
    	 << "<html> <head> <title> Error 404</title> </head> <body> <h1>" 
    	 << "Page not found!"
    	 << " </h1> <p>ask user how to improve it) </p></body></html>";
}

void read_settings(const string & file_name){
	ifstream ifs(file_name);
	string name, eq, value;
	while(ifs) {
		ifs >> name >> eq >> value;
		mapM[name] = stoi(value);
	}
	ifs.close(); 
}

int main() {
	try {
        read_settings("settings.txt");
        enter_page();
        get_files(path("./images"));
	    cout << end_html;


	} 
	catch(...) {
		page_not_found();
	}

	return 0;
}
