#include <stdio.h>
#include <iostream>
#include <fstream>
#include <list>

using namespace std;

#define DEFAULT_NUM_PROCS 4

/* 
 * Comparison functions
 */
int parse_int_for_key(string s, string key, int offset)
{
	int start = s.find(key) + offset;
	char numbers[10];
	int i = 0;
	
	while (s[start] != ' ')
	{
		numbers[i++] = s[start];
		start++;
	}
	numbers[i] = '\0';

	return atoi(numbers);
}

void parse_vector(string s, int *vec)
{
	int start = s.find("vector") + 8;
	for (int i = 0; i < DEFAULT_NUM_PROCS; i++)
	{
		char numbers[10];
		int j = 0;
		
		while (s[start] != ',' && s[start] != ']')
		{
			numbers[j++] = s[start];
			start++;
		}
		numbers[j] = '\0';

		vec[i] = atoi(numbers);
		start += 2;
	}
}

bool compare_pid(const string &first, const string &second)
{
	int pid1 = parse_int_for_key(first, "pid", 4);
	int pid2 = parse_int_for_key(second, "pid", 4);
  	return (pid1 < pid2);
}

bool compare_logical(const string &first, const string &second)
{
	int count1 = parse_int_for_key(first, "logical", 8);
	int count2 = parse_int_for_key(second, "logical", 8);
  	return (count1 < count2);
}

bool compare_vector(const string &first, const string &second)
{
	int vec1[DEFAULT_NUM_PROCS];
	int vec2[DEFAULT_NUM_PROCS];
	parse_vector(first, vec1);
	parse_vector(second, vec2);

	cout << "vector 1: " << vec1[0] << " " << vec1[1] << " " << vec1[2] << " " << vec1[3] << endl;
	cout << "vector 2: " << vec2[0] << " " << vec2[1] << " " << vec2[2] << " " << vec2[3] << endl;

	bool result = true;
	for (int i = 0; i < DEFAULT_NUM_PROCS; i++)
	{
		result = result && (vec1[i] <= vec2[i]);
	}
	if (result)
	{
		for(int i = 0; i < DEFAULT_NUM_PROCS; i++)
		{
			if (vec1[i] < vec2[i])
			{
				cout << "v1 < v2" << endl;
				return true;
			}
		}
	}
	cout << "v2 < v1" << endl;
	return false;
}

/*
 * Search main entry
 */
int main (int argc, const char* argv[])
{	
	// Check arguments
	if (argc < 2){
        cout << "usage: ./search [keyword] [sorted] [comparison] [file]" << endl;
        cout << "i.e. ./search snapshot 1 sorted pid" << endl;
        return 0;
    }

    // Default values
	string keyword = string(argv[1]);
	const char* file = NULL;
	bool sorted = false;
	list<string> matches;
	bool (*comparison)(const string&, const string&) = compare_pid;    

	// Change keyword for convenience
	if (keyword == "message")
	{
		keyword = "message ";
	}

	// Set sorted option if defined
    if (argc >= 3 && argv[2] != NULL)
    {
    	sorted = true;
    }

    // Set comparison method if defined
    if (argc >= 4 && argv[3] != NULL)
    {
    	string comp(argv[3]);
    	if (comp == "pid")
    	{
    		comparison = compare_pid;
    	}
    	else if (comp == "logical")
    	{
    		comparison = compare_logical;
    	}
    	else if (comp == "vector")
    	{
    		comparison = compare_vector;
    	}
    }

    // Check for specific file
    if(argc >=5 && argv[4] != NULL)
    {
    	file = argv[4];
    	ifstream ifile(file);
    	if (!ifile)
    	{
    		cout << "specified file doesn't exist" << endl;
    	}
    }

    // Go through the log files or specified file line by line
    string line;
    if (file == NULL)
    {
	    for (int i = 0; i < DEFAULT_NUM_PROCS; i++)
	    {
	    	char filename[20];
	    	sprintf(filename, "snapshot.%d", i);
	    	ifstream in(filename);

	    	cout << endl << "Searching file: " << filename << endl;

		    if (in.is_open())
		    {
				while (getline(in,line))
				{
					if (line.find(keyword) != string::npos)
					{
						// If not sorting, just print the match, otherwise store it for sorting
						if (!sorted)
						{
							cout << line << endl;
						}
						else 
						{
							matches.push_back(line);
						}
					}
				}
		    }
		}
	}
	else
	{
		fstream in(file);

    	cout << endl << "Searching file: " << file << endl;

	    if (in.is_open())
	    {
			while (getline(in,line))
			{
				if (line.find(keyword) != string::npos)
				{
					// If not sorting, just print the match, otherwise store it for sorting
					if (!sorted)
					{
						cout << line << endl;
					}
					else 
					{
						matches.push_back(line);
					}
				}
			}
	    }
	}

	// Sort with specified comparison method and print
    if (sorted)
    {
    	cout << endl << "Sorting ..." << endl;
    	matches.sort(comparison);

    	for (list<string>::iterator it = matches.begin(); it != matches.end(); it++)
    	{
    		cout << *it << endl;
    	}
    }
    
    return 0;
}

