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
#include <mpi.h>
#include <assert.h>

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
    std::chrono::duration<double, std::milli> IO_time = std::chrono::duration<double, std::milli>::zero();
    auto total_start = std::chrono::system_clock::now();

	if(argc != 3)
	{
		cerr << "Need to pass #filename and  #lines as arguement\n";
	}
	long num_lines = stol(argv[2]);
    string file_name = argv[1];
	int num_procs, curr_rank;
	 MPI_Init(NULL, NULL);
	 MPI_Comm_size(MPI_COMM_WORLD , &num_procs);
	 MPI_Comm_rank(MPI_COMM_WORLD , &curr_rank);
     int block_size = 800;

	 int lines_per_proc = num_lines/num_procs;
     cout << "Lines per proc "<<lines_per_proc<<endl;
	 string delim = "\|";
	 vector<string> final_trg;
     vector<string> final_ref;
     
     if(curr_rank == 0)
	 {
		  //cout << num_procs << " - a -  "<<curr_rank<<endl;
		 ifstream trg,ref;
		 trg.open(file_name+".en");
		 ref.open(file_name+".de");
        
         auto IO_start = std::chrono::system_clock::now();
		//for (int cur_proc_idx = num_procs-1; cur_proc_idx >=0;cur_proc_idx--)
        for (int cur_proc_idx = 0; cur_proc_idx < num_procs;cur_proc_idx++)
		{
			long start_idx = cur_proc_idx * lines_per_proc;
			long end_idx = min(start_idx + lines_per_proc , num_lines);

            int num_blocks = (end_idx - start_idx)/block_size;
            printf("Proc %d Lines per proc %d end %d start %d\n",cur_proc_idx,lines_per_proc,end_idx,start_idx);
            cout << "NUmber of blocks is "<<num_blocks<<endl;
			//Read each line and append to trg and ref strings
			//int resd = (end_idx-start_idx)%block_size;
             
            if(cur_proc_idx > 0)
			{
                MPI_Send(&num_blocks , 1 , MPI_INT , cur_proc_idx , 9 , MPI_COMM_WORLD);
                printf("Done sending\n");
                
                for (int idx = 0 ; idx < num_blocks ; idx++)
                    {
                    string ref_string;
                    string trg_string;
                    
                    int tmp_block_size = block_size;
                    while(tmp_block_size--)
                    {
                    string trg_tmp,ref_tmp;
                    getline(trg , trg_tmp);
                    getline(ref , ref_tmp);
                    ref_string = ref_string+delim+ref_tmp;
                    trg_string = trg_string+delim+trg_tmp;
                    }

                        //cout <<ref_string<<endl;
                        //cout << trg_string<<endl;
                        char *char_trg = new char[trg_string.length()+1];
                        char *char_ref = new char[ref_string.length()+1];
                        strcpy(char_trg , trg_string.c_str());
                        strcpy(char_ref , ref_string.c_str());

                        int length_trg = trg_string.length()+1;
                        int length_ref = ref_string.length()+1;
                        
                        //cout << "Sending to proc "<<cur_proc_idx<<endl;

                        MPI_Ssend(&length_trg , 1 , MPI_INT , cur_proc_idx , 0 , MPI_COMM_WORLD);
                        MPI_Ssend(&length_ref , 1 , MPI_INT , cur_proc_idx , 1 , MPI_COMM_WORLD);
                        MPI_Ssend(char_trg , trg_string.length()+1 , MPI_CHAR , cur_proc_idx , 2 , MPI_COMM_WORLD );
                        MPI_Ssend(char_ref , ref_string.length()+1 , MPI_CHAR , cur_proc_idx , 3 , MPI_COMM_WORLD );

                        delete [] char_trg;
                        delete [] char_ref;
                    }
			}
            else
            {
                for (int idx = 0 ; idx < num_blocks ; idx++)
                    {
                    int tmp_block_size = block_size;
                    while(tmp_block_size--)
                    {
                    string trg_tmp,ref_tmp;
                    getline(trg , trg_tmp);
                    getline(ref , ref_tmp);
                    
                    final_trg.push_back(trg_tmp);
                    final_ref.push_back(ref_tmp);
                    }
                    
                    }
                
            }
               
				//cout << "Done reading files for proc "<<cur_proc_idx<< endl;

				//Make those strings into char arrays and send to appropriate processor
				


		}
        auto IO_end = std::chrono::system_clock::now();
        IO_time += IO_end - IO_start;
	}

	else
	{
        //get number of blocks
        int num_blocks;
        MPI_Recv(&num_blocks, 1, MPI_INT, 0, 9, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        //printf("Received block size for proc %d is %d\n",curr_rank,num_blocks);
        for(int i = 0; i < num_blocks; i++)
        {
		int length_trg , length_ref;

		//get lengths
		MPI_Recv(&length_trg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		MPI_Recv(&length_ref, 1, MPI_INT, 0, 1, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		//cout <<"Proc " << curr_rank<< "Received Lengths from process 0"<<endl;
		
		//cout << length_trg<<" "<<length_ref<<endl;
		
		char* receive_trg = new char[length_trg];
		char* receive_ref = new char[length_ref];

		MPI_Recv(receive_trg , length_trg , MPI_CHAR , 0 , 2 , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
		MPI_Recv(receive_ref , length_ref , MPI_CHAR , 0 , 3 , MPI_COMM_WORLD , MPI_STATUS_IGNORE);
        
               
        string total_trg(receive_trg);
        string total_ref(receive_ref);
        
        vector<string> tmp_final_trg , tmp_final_ref;
        boost::split(tmp_final_trg,total_trg,boost::is_any_of(delim),boost::token_compress_on);
        boost::split(tmp_final_ref,total_ref,boost::is_any_of(delim),boost::token_compress_on);
        
        final_trg.insert(end(final_trg), begin(tmp_final_trg) , end(tmp_final_trg));
        final_ref.insert(end(final_ref), begin(tmp_final_ref) , end(tmp_final_ref));
        
        //cout << "Curr Vector size "<<final_trg.size()<<" "<<final_ref.size()<<endl;
		//cout <<"Proc " << curr_rank << "Received data from process 0"<<endl;
        }
	}
    
    printf("Proc %d ref-size %d  trg-size %d\n",curr_rank,final_ref.size(),final_trg.size());
 assert(final_trg.size() == final_ref.size());
    
    
    double sumd = 0.0;
	for(int i = 0; i < final_trg.size();i++)
	{
		double count = 0.0;
		//getline(trg , cur_trg);
		//getline(ref , cur_ref);

		int trg_len , ref_len;
		StringMap *trg_counts, *ref_counts;

		pair<StringMap*,int> tmp;
		tmp = getFrequncies(final_trg[i]);
		trg_counts = tmp.first;
		trg_len = tmp.second;

		tmp = getFrequncies(final_ref[i]);
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
    
    double overall_BLEU = 0.0;
    MPI_Reduce(&sumd , &overall_BLEU , 1 , MPI_DOUBLE , MPI_SUM , 0 , MPI_COMM_WORLD);
    
    if(curr_rank == 0)
    {
        total_time = std::chrono::system_clock::now() - total_start;
        ofstream outFile;
        outFile.open("Distr_"+file_name+".out",ios::out | ios::app);
        
        cout << "Done Overall BLEU is "<<overall_BLEU/num_lines<<endl;
        cout << "IO time is " << IO_time.count()<<" ms"<<endl;
        cout << "Total Time is "<<total_time.count()<<" ms"<<endl;
	
        outFile << "Done Overall BLEU is "<<overall_BLEU/num_lines<<endl;
        outFile << "IO time is " << IO_time.count()<<" ms"<<endl;
        outFile << "Total Time is "<<total_time.count()<<" ms"<<endl;

    }
 /*
	double sumd = 0.0;
	#pragma omp parallel for reduction(+:sumd)
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
*/	
	MPI_Finalize();
	return 0;
	 }
