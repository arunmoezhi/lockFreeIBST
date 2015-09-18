__inline bool isIFlagSet(struct node* p)
{
	return ((uintptr_t) p & INJECT_BIT) != 0;
}

__inline bool isNull(struct node* p)
{
	return ((uintptr_t) p & NULL_BIT) != 0;
}

__inline bool isDFlagSet(struct node* p)
{
	return ((uintptr_t) p & DELETE_BIT) != 0;
}

__inline bool isPFlagSet(struct node* p)
{
	return ((uintptr_t) p & PROMOTE_BIT) != 0;
}

__inline bool isKeyMarked(unsigned long key)
{
	return ((key & KEY_MASK) == KEY_MASK);
}

__inline struct node* setIFlag(struct node* p)
{
	return (struct node*) ((uintptr_t) p | INJECT_BIT);
}

__inline struct node* setNull(struct node* p)
{
	return (struct node*) ((uintptr_t) p | NULL_BIT);
}

__inline struct node* setDFlag(struct node* p)
{
	return (struct node*) ((uintptr_t) p | DELETE_BIT);
}

__inline struct node* setPFlag(struct node* p)
{
	return (struct node*) ((uintptr_t) p | PROMOTE_BIT);
}

__inline unsigned long setReplaceFlagInKey(unsigned long key)
{
	return (key | KEY_MASK);
}

__inline unsigned long getKey(unsigned long key)
{
	return (key & MAX_KEY);
}

__inline struct node* getAddress(struct node* p)
{
	return (struct node*)((uintptr_t) p &  ~ADDRESS_MASK);
}