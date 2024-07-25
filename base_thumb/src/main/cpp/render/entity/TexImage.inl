

template<typename T>
T TexImage::getImage()
{
    return reinterpret_cast<T>(image);
}