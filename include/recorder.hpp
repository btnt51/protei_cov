#include "commonStructures.hpp"

class IRecoreder {
public:
    virtual void makeRecord(CDR& cdr) = 0;

    virtual ~IRecoreder(){}
};

class FileRecoreder : public IRecoreder {
public:
    void MakeRecord(CDR& cdr);
};
