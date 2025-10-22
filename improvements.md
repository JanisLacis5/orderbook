- use O(logn) data structure (seg tree) for levelData map. if the array size is too large for seg tree (size = maxPrice / tickSize),
go back to a normal map. a further improvement is to normalize prices every market opening, because rarely daily range for an instrument
is larger than 1e6 ticks which could be the largest appropriate seg tree array size