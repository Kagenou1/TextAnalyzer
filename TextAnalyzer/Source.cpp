#include <iostream>
#include <fstream>

#include <vector> 
#include <string>
#include <string_view>

#include <format>
#include <chrono>
#include <future>
#include <queue>

#define THREADS_DEFAULT 4

using namespace std;

class text_analyzer
{
	struct compare
	{
		bool operator()(const int& a, const int& b)
		{
			return a > b;
		}
	};

	using indexes_array = priority_queue<int, vector<int>, compare>;

public:

	indexes_array get_indexes() const
	{
		return indexes;
	}

	int find_substr(const string filename, const string substr, const size_t threads_count = 1)
	{
		std::ifstream f(filename, ios::binary);
		if (!f.is_open() || f.eof())
			return -1;

		if (substr.size() == 0)
			return 0;

		atomic<int> count = 0;

		f.seekg(0, std::ios::end);
		int text_size = f.tellg();
		cout << format("Text size - {} bytes\n", text_size);

		vector < future<int>> threads(threads_count);

		int rem = text_size % threads_count;
		int string_size = text_size / threads_count;
		int pos = 0;

		//flag_write.test_and_set();
		//flag_write.notify_one();

		for (auto& my_thread : threads)
		{
			my_thread = async(launch::async, &text_analyzer::_find_substr, this, filename, substr, pos, string_size + (rem > 0 ? 1 : 0));
			pos += string_size + (rem > 0 ? 1 : 0);
			if (rem > 0)
				rem--;
		}
		for (auto& my_thread : threads)
		{
			my_thread.wait();
			count.fetch_add(my_thread.get());
		}

		return count;
	}

private:

	int _find_substr(const string_view filename, const string_view substr, const int offset = 0, const int bytes_to_read = 0)
	{
		auto begin = chrono::system_clock::now();

		int thread_id = thread_count.load();
		thread_count.fetch_add(1);

		//cout << format("Thread {} from pos {} - {} bytes\n", thread_id, offset, bytes_to_read);

		ifstream file(string(filename).c_str(), ios::binary);
		file.seekg(offset, ios_base::beg);

		size_t substr_size = substr.size();

		if (file.eof() || substr_size == 0)
			return 0;

		int symbols_found = 0;
		atomic<int> count = 0;

		//cout << format("First symbol thread {} - {}\n", thread_id, (char)file.peek());

		for (size_t i = 0; ((bytes_to_read > 0 ? i < bytes_to_read : true) || symbols_found != 0) && !file.eof(); i++)
		{
			if (symbols_found == substr_size)
			{

				while (flag_write.test_and_set(memory_order_acquire)) {}
				indexes.push(offset + i - substr_size);
				flag_write.clear(memory_order_release);				

				count.fetch_add(1);
				symbols_found = 0;
			}
			char ch = file.get();
			if (ch == substr[symbols_found])
				symbols_found++;
			else
				symbols_found = 0;
		}

		auto end = chrono::system_clock::now();

		//cout << format("Symbol after last thread {} - {}", thread_id, (char)file.peek()) << endl;

		file.close();

		cout << format("Thread {} time - {} mcs\n", thread_id, chrono::duration_cast<chrono::microseconds>(end - begin).count());

		return count;
	}

	indexes_array indexes{};
	inline static atomic<int> thread_count = 1;

	atomic_flag flag_write = ATOMIC_FLAG_INIT;
};


int main(int argc, char** argv)
{
	string filename{};
	if (argc >= 2)
		filename = argv[1];
	else
	{
		string_view path_to_exe = argv[0];
		string path_to_folder = string(argv[0]).substr(0, path_to_exe.find_last_of('\\') + 1);
		filename = path_to_folder + "file.txt";
	}
	if (!fstream(filename).is_open())
	{
		cout << "Invalid file path" << endl;
		return -1;
	}
	cout << format("Path to .txt file - {}", filename) << endl;
	string substr{};
	if (argc >= 3)
	{
		substr = string(argv[2]);
		erase(substr, ' ');
	}
	else
		substr = "as";

	int threads_count{};

	if (argc >= 4)
		threads_count = stoi(argv[3]);

	if(threads_count == 0)
		threads_count = THREADS_DEFAULT;


	text_analyzer text1;

	cout << format("Threads count - {}", threads_count) << endl;
	cout << format("String to find - {}", substr) << endl;

	auto begin = chrono::system_clock::now();

	cout << format("Indexes found - {}", text1.find_substr(filename, substr, threads_count)) << endl;

	auto end = chrono::system_clock::now();

	cout << format("Program time - {} mcs\n", chrono::duration_cast<chrono::microseconds>(end - begin).count());

	auto text_indexes = text1.get_indexes();

	/*while(!text_indexes.empty())
	{
		cout << text_indexes.top() << '\t';
		text_indexes.pop();
	}
	cout << endl;*/

	return 0;
}