/* private */
#define SPARSETABLE_GROUP_SIZE 48

struct sparsegroup {
	void *group;		// (small) array of values
	uint16_t num_buckets;	// limits GROUP_SIZE to 64K
	uint8_t bitmap[(SPARSETABLE_GROUP_SIZE - 1) / 8 + 1];	// fancy math is so we round up
	uint8_t deleted[(SPARSETABLE_GROUP_SIZE - 1) / 8 + 1];	// indicates if a position was ever deleted
};

struct sparsegroup_pos {
	ssize_t index;
	ssize_t offset;
};

struct sparsegroup_iter {
	void *val;
	struct sparsegroup_pos pos;
};

#define SPARSEGROUP_VAL(it) ((it).val)
#define SPARSEGROUP_IDX(it) ((it).pos.index)

/* bitmap manipulation */
int sparsegroup_bmtest(const struct sparsegroup *g, ssize_t i);
void sparsegroup_bmset(struct sparsegroup *g, ssize_t i);
void sparsegroup_bmclear(struct sparsegroup *g, ssize_t i);
int sparsegroup_dtest(const struct sparsegroup *g, ssize_t i);
void sparsegroup_dset(struct sparsegroup *g, ssize_t i);
void sparsegroup_dclear(struct sparsegroup *g, ssize_t i);

/* group alloc/free */
void sparsegroup_realloc_group(struct sparsegroup *g, ssize_t n,
				      size_t elt_size);
void sparsegroup_free_group(struct sparsegroup *g, size_t elt_size);

/* indexing */
ssize_t sparsegroup_index_to_offset(const struct sparsegroup *g,
					   ssize_t index);
ssize_t sparsegroup_offset_to_index(const struct sparsegroup *g,
					   ssize_t offset);

/* public functions */

/* constructors */
void sparsegroup_init(struct sparsegroup *g);
void sparsegroup_deinit(struct sparsegroup *g, size_t elt_size);

/* assign, clear */
void sparsegroup_assign_copy(struct sparsegroup *g,
				    const struct sparsegroup *src,
				    size_t elt_size);
void sparsegroup_clear(struct sparsegroup *g, size_t elt_size);
void sparsegroup_clear_tail(struct sparsegroup *g, ssize_t i,
				   size_t elt_size);

/* informative */
ssize_t sparsegroup_count(const struct sparsegroup *g);
ssize_t sparsegroup_size(const struct sparsegroup *g);

/* position-based interface */
void *sparsegroup_find(const struct sparsegroup *g, ssize_t index,
			      struct sparsegroup_pos *pos, size_t elt_size);
void *sparsegroup_insert(struct sparsegroup *g,
				const struct sparsegroup_pos *pos,
				const void *val, size_t elt_size);
void sparsegroup_remove_at(struct sparsegroup *g,
				  const struct sparsegroup_pos *pos,
				  size_t elt_size);
bool sparsegroup_deleted(const struct sparsegroup *g,
				const struct sparsegroup_pos *pos);

/* iteration */
struct sparsegroup_iter sparsegroup_iter_make(const struct sparsegroup
						     *g, size_t elt_size);
void sparsegroup_iter_reset(const struct sparsegroup *g,
				   struct sparsegroup_iter *it,
				   size_t elt_size);
bool sparsegroup_iter_advance(const struct sparsegroup *g,
				     struct sparsegroup_iter *it,
				     size_t elt_size);

