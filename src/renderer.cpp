#include "raylib.h"
#include "renderer.h"
#include <cmath>
#include <algorithm>

Renderer::Renderer(){
    InitWindow(0,0,"");
    windowWidth = GetScreenWidth() * .9;
    windowHeight = GetScreenHeight() * .9;
    CloseWindow();
    InitWindow(windowWidth, windowHeight, "Algoritmo de Dijkstra para Transmilenio");
    radius = windowHeight/80;

    xOffset = 0;
    yOffset = 0;
    xMax = -1;
    yMax = -1;
    mostrarCostos = false;
}

void Renderer::init(Grafo g){
    int numNodos = g.getNodos().getSize();
    for(auto nodo : g.getNodos()){
        xOffset += nodo.value.x;
        if(xMax < nodo.value.x){
            xMax = nodo.value.x;
        }

        yOffset += nodo.value.y;
        if(yMax < nodo.value.y){
            yMax = nodo.value.y;
        }
    }
    xOffset /= numNodos;
    yOffset /= numNodos;
    xMax += xOffset;
    yMax += yOffset;
}

void Renderer::drawNode(NodoDatos nodo, bool start, bool end){
    int inclinacion = 0;
    int offset = 0;
    if(nodo.y == 30 || nodo.y == 70 || nodo.x > 220 || nodo.y < 30){
        inclinacion = 45;
        if(nodo.y == 70 && nodo.x < 147){
            offset = windowHeight * 0.1;
        }
    }
    nodeToScreenCoords(nodo);
    Color color;
    if(start){
        color = GREEN;
    }
    else if(end){
        color = RED;
    }
    else{
        color = BLUE;
    }
    if(nodo.mouseHover){
        auto hsv = ColorToHSV(color);
        color = ColorFromHSV(hsv.x, hsv.y * 0.5f, hsv.z);
    }
    DrawCircle((int)nodo.x, (int)nodo.y, (float)radius, color);
    // DrawText(nodo.nombre.c_str(), int(nodo.x - radius * 0.5), int( nodo.y - radius * 0.5), radius, BLACK);
    DrawTextPro(
                GetFontDefault(),
                nodo.nombre.c_str(),
                Vector2(int(nodo.x - radius * 0.5), int( nodo.y - radius * 0.5)),
                Vector2(offset,0),
                inclinacion,
                radius * 1.5,
                radius * 0.1,
                BLACK
            );
}

double roundDec(double num, int places){
    double multiplier = pow(10, places);
    return ceil(num * multiplier) / multiplier;
}

void Renderer::drawEdge(NodoDatos n1, NodoDatos n2, double dist, bool path){
    Color color = BLACK;
    if(path){
        color = ORANGE;
    }
    nodeToScreenCoords(n1);
    nodeToScreenCoords(n2);
    int xMid = int((n1.x + n2.x)/2);
    int yMid = int((n1.y + n2.y)/2);

    int font = windowHeight/60;
    int fontWidth = font / 2;
    string text = std::to_string(roundDec(dist, 4));
    text.resize(5);
    int ox = int(text.length()/2 * fontWidth);
    int oy = font / 2;
    // DrawText(text.c_str(), xMid - ox, yMid - oy, font, color);
    if(mostrarCostos){
        DrawTextPro(
                    GetFontDefault(),
                    text.c_str(),
                    Vector2(xMid - ox, yMid - oy),
                    Vector2(windowHeight*0.05, 0),
                    45,
                    font,
                    font * 0.1,
                    color
                );
    }
    DrawLine((int)n1.x, (int)n1.y, (int)n2.x, (int)n2.y, color);
}

void Renderer::nodeToScreenCoords(NodoDatos& nodo){
    // int min = std::min(windowWidth, windowHeight);
    // nodo.x += (windowWidth - min)/2;
    nodo.x *= windowWidth / (xMax-115);
    nodo.y *= windowHeight / (yMax-50);
}

void Renderer::setHover(NodoDatos& n){
    NodoDatos temp = n;
    nodeToScreenCoords(temp);
    Vector2 pos = GetMousePosition();
    int dx = int(pos.x - temp.x);
    int dy = int(pos.y - temp.y);
    int dist = dx * dx + dy * dy;
    if(dist < radius * radius){
        n.mouseHover = true;
    }
    else{
        n.mouseHover = false;
    }
}

void setBeginningOrEnd(NodoDatos n, Grafo& g){
    if(n.mouseHover){
        if(IsMouseButtonPressed(0)){
            cout << n.nombre + " is beginning\n";
            g.start = n.nombre;
        }
        else if(IsMouseButtonPressed(1)){
            cout << n.nombre + " is end\n";
            g.end = n.nombre;
        }
    }
}

void Renderer::drawGraph(Grafo& g){
    if(IsKeyPressed(KEY_W)){
        mostrarCostos = !mostrarCostos;
    }
    HashMap<string, DijkstraPath> path;
    int costoTotal = 0;
    if(!g.start.empty() && !g.end.empty()){
        path = g.dijkstra();
        costoTotal = path[g.end].dist;
    }
    int fontSize = windowHeight * 0.03;
    DrawText(
            (string("Inicio: ") + g.start).c_str(),
            windowWidth * 0.01,
            windowHeight * 0.8,
            fontSize,
            BLACK
    );
    DrawText(
            (string("Fin: ") + g.end).c_str(),
            windowWidth * 0.01,
            windowHeight * 0.8 + fontSize * 2,
            fontSize,
            BLACK
    );
    DrawText(
            (string("Costo total: ") + std::to_string(costoTotal)).c_str(),
            windowWidth * 0.01,
            windowHeight * 0.8 + fontSize * 4,
            fontSize,
            BLACK
    );
    for(auto nodo : g.getNodos()){
        string nodoNombre = nodo.value.nombre;
        for(auto vecino : g.getVecinos(nodo.key)){
            NodoDatos nodoVecino = g.getNodo(vecino.nombre);
            if(path[nodoNombre].from == nodoVecino.nombre || path[nodoVecino.nombre].from == nodoNombre){
                drawEdge(nodo.value, nodoVecino, vecino.dist, true);
            }
            else{
                drawEdge(nodo.value, nodoVecino, vecino.dist, false);
            }
        }
    }
    for(auto nodo : g.getNodos()){
        if(nodo.key == ""){
            continue;
        }
        setHover(nodo.value);
        setBeginningOrEnd(nodo.value, g);
        bool start = g.start == nodo.value.nombre;
        bool end = g.end == nodo.value.nombre;
        drawNode(nodo.value, start, end);
    }
}

        
void Renderer::renderProcess(Grafo& g){
    BeginDrawing();
        ClearBackground(RAYWHITE);
        drawGraph(g);
    EndDrawing();
}