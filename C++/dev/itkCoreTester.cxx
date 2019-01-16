#include "itkImage.h"
#include <iostream>

int main(int argc, char *argv[])
{
    std::cout << "GlobalDefaultNumberOfThreads=" <<
        itk::MultiThreader::GetGlobalDefaultNumberOfThreads() << std::endl;

    std::cout << "ThreadPool threads=" <<
        itk::ThreadPool::GetInstance()->GetNumberOfCurrentlyIdleThreads() << std::endl;

    return EXIT_SUCCESS;
}
