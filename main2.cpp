#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <random>

using namespace std;

class Task
{

	int jobID;
	int machineID;
	int duration;
    int start_time;

    public:
    
    Task(int d, int jID)
    {
        this->duration = d;
        this->jobID = jID;
    }

    Task(int m, int d, int jID)
    {
        this->machineID = m;
        this->duration = d;
        this->jobID = jID;
    }

    int get_machine_ID() {return machineID;}
    int get_duration() {return duration;}
    int get_job_ID() {return jobID;}
    int get_start_time() {return start_time;}

    void set_duration(int d) {this->duration = d;}
    void set_machine_ID(int m) {this->machineID = m;}
    void set_jobID(int jID) {this->jobID = jID;}
    void set_start_time(int st)  {this->start_time = st;}

    void reset() {this->start_time = 0;}
};

class Job {

    friend class Task;
    int ID;
	std::vector<Task> tasks = {};
	int curr_task = 0;
	int end_time = 0;

public:

    Job (int i)
    {
        this->ID = i;
    }

    void set_jobID(int value) { ID=value; }

    void add_Task(Task t)
    {
        tasks.push_back(t);
    }

    int get_ID() {return this->ID; }

    Task& get_Task(int i) {return this->tasks[i]; }

    std::vector<Task> get_tasks() {return this->tasks; }

    int get_curr_task() {return this->curr_task;}
    void next_task() {curr_task++;}
    int get_end_time() {return end_time;}
    void set_end_time(int time) {end_time = time;}
    int get_number_of_tasks() {return this->tasks.size();}
    
    void reset()
    {
        this->end_time = 0;
        this->curr_task = 0;
        for (auto & task : this->tasks)
        {
            task.reset();
        }
    }

};

class Machine{

    friend class Task;
    int machineID;
    bool free = true;
    int end_time = 0;
    int occupant_ID;
    std::vector<Task> tasks = {};

public:

    Machine(int ID){machineID = ID;}
    bool check_if_free(int time) 
    {
        if (not free && time >= end_time)
        {
            free = true;
        }
        return free;
    }
    void set_end_time(int time) {end_time = time;}
    void set_busy() {free=false;}
    int get_end_time() {return end_time;}
    int get_machine_id() {return machineID;}
    std::vector<Task> get_tasks() {return this->tasks; }
};

class Factory {

    friend class Job;
    int number_of_jobs;
    int number_of_machines;
    vector<Job> jobs;
    vector<Machine> machines;
    int exec_len = 0;

public:

    Factory(const string&, char, int);

    void set_number_of_jobs(int value) { number_of_jobs = value; }
    void set_number_of_machines(int value) { number_of_machines = value; }
    void set_exec_len(int value) {exec_len = value;}

    int get_number_of_jobs() { return number_of_jobs; }
    int get_number_of_machines() { return number_of_machines; }
    int get_exec_len() {return exec_len; }

    void add_Job(Job j)
    {
        jobs.push_back(j);
    }

    Job &get_Job(int i) {return this->jobs[i];}

    int greedy_solve(vector<int> jobs_order)
    {
        int time = 0;
        int next_time = 0;
        vector<Machine> machines;
        for (int i=0; i<number_of_machines; i++)
        {
            machines.push_back(Machine(i));
        }
        int temp, task_n;
        int done = 0;
        while (done >=time) 
        {
            next_time = (int) time;
            for (auto k : jobs_order)
            {
                Job &j = this->jobs[k];
                task_n = j.get_curr_task();
                if (task_n >= number_of_machines) { continue; }
                auto &t = j.get_Task(task_n);
                temp = t.get_machine_ID();
                if (machines[temp].check_if_free(time) && j.get_end_time() <= time) 
                {                      
                    t.set_start_time(time);
                    machines[temp].set_end_time(t.get_duration() + time);
                    machines[temp].set_busy(); 
                    j.next_task();
                    j.set_end_time(t.get_duration() + time);
                    if (done < t.get_duration() + time) { done = t.get_duration() + time; }
                    Machine(t.get_machine_ID()).get_tasks().push_back(t);
                }
                if ( ( machines[t.get_machine_ID()].check_if_free(time) &&
                        machines[t.get_machine_ID()].get_end_time() < next_time ) ||
                        next_time == time)
                    {
                        if(time <= machines[t.get_machine_ID()].get_end_time()) {next_time = machines[t.get_machine_ID()].get_end_time();}
                    }
                if (j.get_end_time() < next_time && j.get_end_time() > time) {next_time = j.get_end_time();}
            }
            if (next_time != time) { time = next_time; }
            else { ++time; }
        }
        for (auto &j : this->jobs)
        {
            if (exec_len < j.get_end_time()) { exec_len = j.get_end_time(); }
        }
        return exec_len;
    }

    void reset()
    {
        this->exec_len = 0;
        for (auto & job : this->jobs){
            job.reset();
        }
    }

    void opti_solve(int time)
    {   
        chrono::seconds duration(time);
        chrono::time_point<chrono::system_clock> end = chrono::system_clock::now() + duration;
        auto start = chrono::steady_clock::now();

        vector<int> jobs_order = {};
        int i=0;
        while (i < this->get_number_of_jobs()) {jobs_order.push_back(i); ++i;}
        
        auto randomizer = default_random_engine {};
        shuffle(jobs_order.begin(),jobs_order.end(), randomizer);
        int min = this->greedy_solve(jobs_order);
        vector<int> best_jobs_order = jobs_order;

        int res;
        while(chrono::system_clock::now() < end) 
        {
            this->reset();          
            shuffle(jobs_order.begin(),jobs_order.end(), randomizer);
            res = this->greedy_solve(jobs_order);
            int local_min = res;
            int local_start_res = res;
            if (res < min) 
            {
                min = res; 
                best_jobs_order = jobs_order; 
            }
            vector<int> local_order = jobs_order; 
            bool better = true;
            while (better == true && chrono::system_clock::now() < end)
            {
                
                next_permutation(jobs_order.begin(), jobs_order.end());
                this->reset();
                res = this->greedy_solve(jobs_order);
                if (res < local_min) 
                {
                    local_min = res;
                    if (res < min) 
                    {
                        min = res; 
                        best_jobs_order = jobs_order; 
                    }
                } 
                else {better = false;}
                
            }
            jobs_order = local_order;
            local_min = local_start_res;
            better = true;

            while (better == true && chrono::system_clock::now() < end)
            {   
                
                prev_permutation(jobs_order.begin(), jobs_order.end());
                this->reset();
                res = this->greedy_solve(jobs_order);
                if (res < local_min) 
                {
                    local_min = res;
                    if (res < min) 
                    {
                        min = res; 
                        best_jobs_order = jobs_order;
                    }
                }
                else {better = false;}
            }   
                
        }
        this->reset();
        this->greedy_solve(best_jobs_order);
        
    }
    

    void print() {

        cout << "Factory with " << get_number_of_jobs() << " jobs on " << get_number_of_machines() << " machines "
             << endl;
        for (auto &j : this->jobs) 
        {
            cout << "Job number " << j.get_ID() << endl << "Machine ID, Duration" << endl;
            for (auto &t : j.get_tasks()) 
            {
                cout << t.get_machine_ID() << " " << t.get_duration() << endl;
            }
            cout << endl;
        }
    }

    void print_answer()
    {
        cout << exec_len << endl;
        for (auto &j : this->jobs) 
        {
            for (auto &t : j.get_tasks())
            {
                cout << t.get_start_time() << ' ';
            }
            cout << endl;
        }
    }

    void out_answer(string path)
    {
        ofstream output;
        output.open(path.c_str());
        if (output.is_open())
        {
            output << exec_len << endl;
            for (auto &j : this->jobs) 
                {
                for (auto &t : j.get_tasks())
                    {
                        output << t.get_start_time() << ' ';
                    }
                    output << endl;
                }
            
            output.close();
        }
        else
        {
            cerr << "blad eksportu odpowiedzi do pliku";
        }
        
    }
    int lower_bound()
    {   
        int maxjobl = 0;
        for (auto &j : this->jobs) {
            int jobl = 0;
            for (auto &t : j.get_tasks())
            {
                jobl += t.get_duration();
            }
            if (jobl > maxjobl) {maxjobl = jobl; }

    } 
    return maxjobl;
    }
};

Factory::Factory(const string& path, char instance = 'o', const int maxjobs = -1)
{
    ifstream input;
    string line;

    stringstream ss;
    int temp;

    input.open(path, fstream::in);
    if (input.is_open())
    {
        
        getline(input, line);
        ss << line;
        ss >> this->number_of_jobs;
        if (maxjobs >=0 && maxjobs < number_of_jobs) {this->number_of_jobs = maxjobs ;}
        ss >> this->number_of_machines;
        while(ss >> temp);
        ss.clear();
        
        int jobID = 0;
        
        if (instance == 't' )
        {   
            bool machines_read = false;
            while (getline(input, line) && ((maxjobs == -1) || jobID < maxjobs))
            {   
                if (line.find("Times") != string::npos) {line.clear();  continue ;}

                if (line.find("Machines") != string::npos) { line.clear(); jobID = 0; machines_read = true; break; }
                
                ss<<line;
                Job j(jobID);

                while(ss >> temp)
                {
                    Task task(temp, jobID);              
                    j.add_Task(task);
                }
                this->add_Job(j);

                ++jobID;
                ss.clear();

            }

            while (machines_read == false) 
            {   
                getline(input, line);
                if (line.find("Machines") != string::npos) { line.clear(); jobID = 0; machines_read = true; break; }
            }

            while (getline(input, line) && ((maxjobs == -1) || jobID < maxjobs))
            {
                
                ss<<line;
                int taskID=0;              
                while(ss >> temp)
                {
                    int mID = temp - 1;
                    this->get_Job(jobID).get_Task(taskID).set_machine_ID(mID);
                    taskID++;
                }

                ++jobID;
                ss.clear();           
            
            }

        }

        else if (instance == 'o')
        {

            while (getline(input, line) && ((maxjobs == -1) || jobID < maxjobs))
            {
                if (line.empty()) {continue;}
                ss<<line;
                Job j(jobID);

                while(ss >> temp)
                {
                    int machine = temp;
                    int duration;
                    ss>>duration;
                    Task task(machine, duration, jobID);
                    j.add_Task(task);
                }
                if(j.get_number_of_tasks() > 0){
                    this->add_Job(j);
                }

                ++jobID;
                ss.clear();

            }
        }
	else 
        {
            cerr << "Blad - niepoprawnie podany format pliku, o - orlib (format J.E.Beasleya), t - tailard" << endl;
            throw;
        }           
     
        input.close();
    } 
    else
    {
        cerr << "Blad otwarcia pliku" << endl;
        throw;
    }
}



int main(int argc, char *argv[])
{

    if (argc < 3) 
    {
        cerr << "Prosze podac plik zrodlowy, typ danych (o - orlib (format J.E.Beasleya), t - tailard) oraz opcjonalnie liczbe prac do wczytania (domyslnie wszystkie)" << endl;
        cerr << "np: ./main zrodlo.txt o" << endl ;
        return -1;
    }
      
    Factory factory(argv[1], *argv[2], ((argv[3]) ? (stoi(argv[3])) : -1));  

    factory.opti_solve(30);
    
    factory.print_answer();

    return 0;
}