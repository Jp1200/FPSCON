// Fpscpp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <algorithm>
using namespace std;
#include <chrono>
#include <Windows.h>
#include <vector>
int nScreenHeight = 40;
int nScreenWidth = 120;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;
float fSpeed = 5.0f;
float fFOV = 3.141 / 4.0;
float fDepth = 16.0;
int main()
{
    // create screen buffer
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwbytesWritten = 0;
    
    wstring map;

    map += L"################";
    map += L"#...........#..#";
    map += L"#...........#..#";
    map += L"#..........##..#";
    map += L"#..............#";
    map += L"#...#..........#";
    map += L"#..............#";
    map += L"#..........#...#";
    map += L"#..............#";
    map += L"#####..........#";
    map += L"#..............#";
    map += L"#.........##...#";
    map += L"#.#........#...#";
    map += L"#...##.........#";
    map += L"#....#.........#";
    map += L"################";

    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();
    while (1) {
        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElpasedTime = elapsedTime.count();

        //Controls

        if (GetAsyncKeyState((unsigned short) 'A') & 0x8000)
        {
            fPlayerA -= (0.9f)* fElpasedTime;
        }
        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
        {
            fPlayerA += (0.9f) * ( fElpasedTime);
        }
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            fPlayerX += sinf(fPlayerA) * fSpeed * fElpasedTime;;
            fPlayerY += cosf(fPlayerA) * fSpeed * fElpasedTime;;
            if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
            {
                fPlayerX -= sinf(fPlayerA) * fSpeed * fElpasedTime;;
                fPlayerY -= cosf(fPlayerA) * fSpeed * fElpasedTime;;
            }
        }

        // Handle backwards movement & collision
        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            fPlayerX -= sinf(fPlayerA) * fSpeed * fElpasedTime;;
            fPlayerY -= cosf(fPlayerA) * fSpeed * fElpasedTime;;
            if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
            {
                fPlayerX += sinf(fPlayerA) * fSpeed * fElpasedTime;;
                fPlayerY += cosf(fPlayerA) * fSpeed * fElpasedTime;;
            }
        }
        for (int x = 0; x < nScreenWidth; x++) {
            float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

            float fDistanceToWall = 0;
            bool bHitWall = false;
            bool bBoundary = false;

            float fEyeX = sinf(fRayAngle);
            float fEyeY = cosf(fRayAngle);
            while (!bHitWall) {
                fDistanceToWall += 0.1f;

                int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);
                // Test if ray is out of bounds 
                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                {
                    bHitWall = true;
                    fDistanceToWall = fDepth;
                }
                else
                {
                    if (map[nTestY * nMapWidth + nTestX] == '#')
                    {
                        bHitWall = true;
                        vector<pair<float, float>> p; //distance, dot

                        for (int tx = 0; tx < 2; tx++)
                            for (int ty = 0; ty < 2; ty++)
                            {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX; 
                                   float d = sqrt(vx * vx + vy * vy);
                                   float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                   p.push_back(make_pair(d, dot));
                            }
                        sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right) {return left.first < right.first;  });
                        
                        float fBound = 0.01;
                        if (acos(p.at(0).second) < fBound) bBoundary = true;
                        if (acos(p.at(1).second) < fBound) bBoundary = true;
                        if (acos(p.at(2).second) < fBound) bBoundary = true;
                    }
                }
            }

            //Calulate Distance to Ceiling and floor 
            int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
            int nFloor = nScreenHeight - nCeiling;

            short nShade = ' ';
            if (fDistanceToWall <= fDepth / 4.0f) nShade = 0x2588;
            else if (fDistanceToWall < fDepth / 3.0f) nShade = 0x2593;
            else if (fDistanceToWall < fDepth / 2.0f) nShade = 0x2592;
            else if (fDistanceToWall < fDepth ) nShade = 0x2591;
            else nShade = ' ';

            if (bBoundary) nShade = ' ';

            for (int i = 0;  i < nScreenHeight; i++)
            {
                if (i <= nCeiling) 
                    screen[i * nScreenWidth + x] = ' ';
                
                else if( i > nCeiling && i <= nFloor)
                
                    screen[i * nScreenWidth + x] = nShade;
                else
                {
                    float b = 1.0f - (((float)i - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
                    if (b < 0.25) nShade = '#';
                    else if (b < 0.5) nShade = 'x';
                    else if (b < 0.75) nShade = '.';
                    else if (b < 0.9) nShade = '-';
                    else                nShade = ' ';
                    screen[i*nScreenWidth + x] = nShade;
                }
            }

        }
        // Display Stats
        swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElpasedTime);

        // Display Map
        for (int nx = 0; nx < nMapWidth; nx++)
            for (int ny = 0; ny < nMapWidth; ny++)
            {
                screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + nx];
            }
        screen[((int)fPlayerX + 1) * nScreenWidth + (int)fPlayerY] = 'P';
        screen[nScreenWidth * nScreenHeight - 1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwbytesWritten);
        
    }
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
