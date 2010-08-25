
struct hash_link {
  int	key;
  struct hash_link *next;
  void	*data;
};

struct hash_header {
  int	rec_size;
  int	table_size;
  int	*keylist, klistsize, klistlen; /* this is really lame,
					  AMAZINGLY lame */
  struct hash_link	**buckets;
};


#define WORLD_SIZE 50000
