#include "Delete.hpp"

Delete::Delete(HTTP const &other) : HTTP(other)
{
}

Delete::Delete(Delete const &other) : HTTP(other)
{
    (void)other;
    return ;
}

Delete    &Delete::operator=(Delete const &other)
{
   (void)other;
   return (*this);
}

Delete::~Delete()
{
}

void Delete::request_delete()
{
    std::string path;

    path = TransformToString() << pwd << URI;
    //verifier que la personne bien le droit de supprimer le fichier demande 
    if (std::remove(path.c_str()))
    {
        throw (ErrorCodes(404));
    }
}