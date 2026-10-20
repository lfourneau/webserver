#ifndef CONCATENATE_HPP
# define CONCATENATE_HPP

# include <string>
# include <sstream>

class TransformToString
{
    public:
        TransformToString() {};
        TransformToString(TransformToString const &other)
        {
            this->sentence << other.sentence.str();
        };
        TransformToString &operator=(TransformToString const &other)
        {
            if (this != &other)
            {
                this->sentence.str("");
                this->sentence.clear();
                this->sentence << other.sentence.str(); 
            }
            return (*this);
        };
        ~TransformToString(){};
        template <typename T> TransformToString &operator<<(T content)
        {
            this->sentence << content;
            return (*this);
        }
        operator std::string() const 
        {
            return this->sentence.str();
        };
    private:
        std::ostringstream sentence;
};

#endif