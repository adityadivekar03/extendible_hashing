#include <bits/stdc++.h>

using namespace std;

#define BUCKET_SQUASH 1
#define DIRECTORY_COMPRESSION 1

struct bucket_elem{
int data;
int misc;
};

struct dir_elem{
int local_depth;
std::vector<bucket_elem>* ptr;
int misc;
};

class HashVector{
public:
	vector<dir_elem> dir;
	int global_depth;
	int perbucket;

	/* Functions. */
	void initialize(int, int);
	void insert(int);
	void split_and_insert(int,int);
	int search(int);
	int delete_elem(int);
	void status(void);
	int hash(int a) {return a%(int)pow(2,global_depth); }
	int old_hash(int a){ return a%(int)pow(2,global_depth-1); }
	int power(int a) { return pow(2,a); }
};

void HashVector::initialize(int initial_depth, int perbucket)
{
	HashVector::global_depth = initial_depth;
	HashVector::perbucket = perbucket;
	/* Initialize the directory with 2^n dentries. */
	for(int i=0;i<pow(2,global_depth);i++)
	{
		dir.push_back({global_depth, NULL, -1});
	}
	/* Make each dentry point to a bucket. Each bucket is a vector in itself. */
	for (int i=0;i<dir.size();i++)
	{
		/* Create one bucket. */
		dir[i].ptr = new vector<bucket_elem>();
		// vector<bucket_elem> elem;
		// elem.push_back({-1,-1});
		// dir[i].ptr = &elem;
	}
}

void HashVector::insert(int data)
{
	int key = HashVector::hash(data);
	if(!dir[key].ptr->empty() && dir[key].ptr->size()>=perbucket)
	{
		HashVector::split_and_insert(data, key);
	}
	else
	{
		bucket_elem elem = {data,-1};
		(dir[key].ptr)->push_back(elem);
	}
}

void HashVector::split_and_insert(int data, int key)
{
	/* Check if directory expansion is needed. */
	if(dir[key].local_depth==global_depth)
	{
		/* Create a new bucket, the split bucket. */
		vector<bucket_elem>* split_bucket = new vector<bucket_elem>();
		int ctr=0; //counter for split bucket.
		/* Change hash function by increasing the global depth. */
		global_depth+=1;
		
		/* Rearrange the data between the original and the new bucket.*/
		vector<bucket_elem> *original_bucket = dir[key].ptr;
		vector<bucket_elem>::iterator it = original_bucket->begin();

		for ( ; it != original_bucket->end(); )
		{
			if (hash((*it).data)!=key)
			{
				split_bucket->push_back({it->data,it->misc});
				it = original_bucket->erase(it);
			}
			else
			{
				++it;
			}
		}
		/* Double the directory size. */ 
		dir[key].local_depth+=1;
		/* Rearrange pointers. */
		dir.resize(pow(2,global_depth));
		for(int i=pow(2,global_depth-1); i<pow(2,global_depth); i++)
		{
			dir[i] = dir[i-pow(2,global_depth-1)];
			if(old_hash(i)==key)
				dir[i].ptr = split_bucket;
		}
		/* Now insert the data that caused the split. */
		HashVector::insert(data);
	}

	/* No directory expansion needed. */
	else if(dir[key].local_depth<global_depth)
	{
		/* Create a new bucket, the split bucket. */
		vector<bucket_elem>* split_bucket = new vector<bucket_elem>();
		int ctr=0; //counter for split bucket.
		/* Rearrange the data between the original and the new bucket.*/
		vector<bucket_elem> *original_bucket = dir[key].ptr;
		vector<bucket_elem>::iterator it = original_bucket->begin();

		for ( ; it != original_bucket->end(); )
		{
			if (hash((*it).data)!=key)
			{
				split_bucket->push_back({it->data,it->misc});
				it = original_bucket->erase(it);
			}
			else
			{
				++it;
			}
		}
		/* Set pointer for the new bucket. */
		dir[key+pow(2,global_depth-1)].ptr = split_bucket;
		/* Increase local depths. */
		dir[key].local_depth+=1;
		dir[key+pow(2,global_depth-1)].local_depth+=1;
	}
	else
		cout<<"Should never execute\n";
}

int HashVector::search(int data)
{
	int key = hash(data);
	vector<bucket_elem>::iterator it = dir[key].ptr->begin();
	for(; it!=dir[key].ptr->end(); it++)
	{
		if( it->data == data)
		{	
			cout<<data<<" exists in the table at bucket number "<<key<<endl;
			return key;
		}
	}
	cout<<data<<" was not found in the Hash Table"<<endl;
	return -1;
}

int HashVector::delete_elem(int data)
{
	int key = HashVector::search(data);
	if(key==-1)
	{
		cout<<"The given data does not exist in the table"<<endl;
		return 1;
	}
	else
	{
		vector<bucket_elem>::iterator it= dir[key].ptr->begin();
		for(; it!=dir[key].ptr->end();)
		{	
			if(it->data==data)
				it = dir[key].ptr->erase(it);
			else
				++it;
		}
	}
	/* Now perform maintainence functions. */
	/* 1. Bucket Squashing. */
	if(BUCKET_SQUASH)
	{
		int halfsize = power(dir[key].local_depth - 1);
		int mirror = (key>halfsize)? key- halfsize: key+halfsize;
		vector<bucket_elem>* p= dir[key].ptr;
		vector<bucket_elem>* q= dir[mirror].ptr;
		if(p->size() + q->size() <= perbucket)
		{
			for(int i=0;i<q->size();i++)
			{
				p->push_back(q->at(i));
			}
			// delete dir[mirror].ptr;
			dir[mirror].ptr = p;
		}
	}
	/* Directory Compression. */
	if(DIRECTORY_COMPRESSION)
	{
		int halfsize = power(global_depth-1);
		for(int i=0;i<halfsize;i++)
		{
			if(dir[i].ptr!=dir[i+halfsize].ptr)
				return 1;
		}
		/* Return not executed implies that the directory can be compressed. */
		for(int i=0;i<halfsize;i++)
			dir.pop_back();
		cout<<"Halved the directory size"<<endl;
	}
	return 0;
}

void HashVector::status()
{
	/* Contents of the HashTable. */
	/* Print the global depth first. */
	cout<<"Global Depth is "<<global_depth<<endl;
	/* Print the local depth and the contents of that bucket. */
	for (int i=0;i<dir.size(); i++)
	{
		cout<<"Local depth of bucket "<<i<<" is "<<dir[i].local_depth<<endl;
		if(dir[i].ptr->size()>0)
		{
			vector<bucket_elem>::iterator it = dir[i].ptr->begin();
			cout<<"Keys in this bucket are \n";
			// for(int j=0;j<dir[i].ptr->size();j++)
			for(;it != dir[i].ptr->end();it++)
			{
				cout<<" -> "<<(*it).data<<" ";
			}
			cout<<endl;
		}
		else
			cout<<"Bucket is empty"<<endl;
	}
}

int main()
{
	HashVector HV;
	HV.initialize(2, 3);
	HV.insert(1);
	HV.insert(1);
	HV.insert(9);
	HV.insert(0);
	HV.insert(7);
	HV.insert(6);
	HV.insert(14);
	HV.insert(17);
	HV.delete_elem(1);
	HV.delete_elem(9);
	HV.delete_elem(0);
	HV.delete_elem(6);
	HV.delete_elem(17);
	HV.status();
	return 0;
}