#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <iterator>
#include <regex>
#include <omp.h>
#include <chrono>
#include <boost/algorithm/string.hpp>

using namespace std;

typedef unordered_map<string , int> StringMap;


pair<StringMap*,int> getFrequncies(string test)
{

	vector<string> result_ref;
    boost::split(result_ref,test,boost::is_any_of("\t "),boost::token_compress_on);
	/*
    regex re("\\s+");
	sregex_token_iterator iter(test.begin(),test.end(),re,-1);
	sregex_token_iterator end;
	for ( ; iter != end; ++iter)
	{
		result_ref.push_back(*iter);
	}
    */

	
	StringMap*  freq_ref = new StringMap();
	for (string tmp : result_ref)
	{
		if(freq_ref->find(tmp) == freq_ref->end())
		{
			//cout<< tmp <<"-first"<< endl;
			(*freq_ref)[tmp] = 1;
		}
		else
		{
			//cout<< tmp <<"-multiple"<< endl;
			(*freq_ref)[tmp] += 1;
		}
	}
	pair<StringMap*,int> ret = make_pair(freq_ref , result_ref.size());
	return ret;

}
int main(int argc , char* argv[])
{
    std::chrono::duration<double, std::milli> total_time = std::chrono::duration<double, std::milli>::zero();
    auto total_start = std::chrono::system_clock::now();
	if(argc != 3)
	{
		cerr << "Need to pass #filename(minus suffix) and #lines as arguments\n";
	}
	string file_name = argv[1];
    long num_lines = stol(argv[2]);
	long lines_per_node = 8000000;
	long num_IO_reads = (num_lines-1)/lines_per_node + 1;

	ifstream trg,ref;
	trg.open(file_name+".en");
	ref.open(file_name+".de");
	double overall_BLEU = 0.0;
    std::chrono::duration<double, std::milli> IO_time = std::chrono::duration<double, std::milli>::zero();
    
	for (int idx = 0 ; idx < num_IO_reads ; idx++)
	{
        auto IO_start = std::chrono::system_clock::now();
        
		vector<string> ref_sens;
		vector<string> trg_sens;
		long start_idx = idx*lines_per_node;
		long end_idx = min (start_idx + lines_per_node , num_lines);
		for(int i = start_idx; i < end_idx ;i++)
		{
			string trg_tmp,ref_tmp;
			getline(trg , trg_tmp);
			getline(ref , ref_tmp);
			ref_sens.push_back(ref_tmp);
			trg_sens.push_back(trg_tmp);
		}
        
        auto IO_end = std::chrono::system_clock::now();
        IO_time += IO_end - IO_start;
        
		cout << "Done reading files!"<< endl;
		string cur_trg , cur_ref;
        
        double sumd = 0.0;
		for(int i = 0; i < end_idx-start_idx;i++)
		{
			double count = 0.0;
			//getline(trg , cur_trg);
			//getline(ref , cur_ref);

			int trg_len , ref_len;
			StringMap *trg_counts, *ref_counts;

			pair<StringMap*,int> tmp;
			tmp = getFrequncies(trg_sens[i]);
			trg_counts = tmp.first;
			trg_len = tmp.second;

			tmp = getFrequncies(ref_sens[i]);
			ref_counts = tmp.first;
			ref_len = tmp.second;

			for (auto idx = trg_counts->begin();idx != trg_counts->end();idx++)
			{
				if(ref_counts->find(idx->first) != ref_counts->end())
					count += min(idx->second , (*ref_counts)[idx->first]);
			}

			//cout << count <<"--"<<i<< endl;
			//overall_BLEU += count/(double)trg_len;
            sumd = sumd + count/(double)trg_len;
			delete trg_counts;
			delete ref_counts;
		}
        //cout <<sumd<<endl;
        overall_BLEU += sumd;
	}

    total_time = std::chrono::system_clock::now() - total_start; 
    
    ofstream outFile;
    outFile.open("Serial_"+file_name+".out",ios::out | ios::app);
    
    outFile << "Done Overall BLEU is "<<overall_BLEU/num_lines<<endl;
    outFile << "IO time is " << IO_time.count()<<" ms"<<endl;
    outFile << "Total Time is "<<total_time.count()<<" ms"<<endl;
    
	cout << "Done Overall BLEU is "<<overall_BLEU/num_lines<<endl;
	cout << "IO time is " << IO_time.count()<<" ms"<<endl;
    cout << "Total Time is "<<total_time.count()<<" ms"<<endl;
   

	return 0;
}

