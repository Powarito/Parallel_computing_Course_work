from dataclasses import dataclass
from functools import total_ordering

@dataclass
@total_ordering
class word_entry:
    def __init__(self, file_id = 0, position = 0):
        self.file_id = file_id
        self.position = position

    def __eq__(self, other):
        return self.file_id == other.file_id and self.position == other.position

    def __lt__(self, other):
        return (self.file_id < other.file_id or (self.file_id == other.file_id and self.position < other.position))
    
def hash_word_entry(entry):
    h1 = hash(entry.file_id)
    h2 = hash(entry.position)
    return h1 ^ (h2 << 1)