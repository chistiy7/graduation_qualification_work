#include "field.hpp"


Field::Field(unsigned _width, unsigned _height)
: width(_width),
height(_height) {
    // Setting new grid
    grid = new IndexedCell[width*height];
    // Clearing it
    for (int i=0; i < width*height; ++i) {
        grid[i].setType(CellType::None);
    }
}

Field::Field(const Field& _field)
: width(_field.width),
height(_field.height)  {
    // Setting new grid
    grid = new IndexedCell[width*height];
    // Copying previous field into new
    memcpy(grid, _field.grid, height*width*sizeof(grid[0]));
}

Field::~Field() {
    delete[] grid;
}

Field& Field::operator=(const Field& _field) {
    // Clear previous
    delete[] grid;

    // Crete new
    width = _field.width;
    height = _field.height;
    // Setting new grid
    grid = new IndexedCell[width*height];
    // Copying previous field into new
    memcpy(grid, _field.grid, height*width*sizeof(grid[0]));

    return *this;
}

void Field::reset(unsigned _width, unsigned _height) {
    delete[] grid;
    width = _width;
    height = _height;
    grid = new IndexedCell[width*height];
    // Clearing it
    for (int i=0; i < width*height; ++i) {
        grid[i].setType(CellType::None);
    }
}

void Field::setWidth(unsigned _width) {
    // Creating new array of cells
    IndexedCell* newGrid = new IndexedCell[height*_width];
    for (int y=0; y < height; ++y) {
        // Copying line (or as much as can) to new array
        memcpy(newGrid+y*_width, grid+y*width, std::min(width, _width)*sizeof(IndexedCell));
    }
    // Setting new options
    delete[] grid;
    grid = newGrid;
    width = _width;
}

void Field::setHeight(unsigned _height) {
    // Creating new array of cells
    IndexedCell* newGrid = new IndexedCell[_height*width]{};
    
    // Copying as much elements as we could
    if (_height > height) {
        memcpy(newGrid, grid, height*width*sizeof(IndexedCell));
    } else {
        memcpy(newGrid, grid, _height*width*sizeof(IndexedCell));
    }

    // Setting new options
    delete[] grid;
    grid = newGrid;
    height = _height;
}

unsigned Field::getWidth() {
    return width;
}

unsigned Field::getHeight() {
    return height;
}

IndexedCell& Field::getCell(sf::Vector2i pos) {
    return grid[pos.x + pos.y*width];
}

IndexedCell& Field::operator[](sf::Vector2i pos) {
    return grid[pos.x + pos.y*width];
}

IndexedCell& Field::operator[](unsigned _pos) {
    return grid[_pos];
}

void Field::save(std::ofstream& fout) {
    fout << width << " x " << height << '\n';

    // Writing grid data
    for (int y=0; y < height; ++y) {
        for (int x=0; x < width; ++x) {
            fout << getCell({x, y}).saveAs();
        }
        fout << '\n';
    }
}

void Field::load(std::ifstream& fin) {
    // Clearing previous data
    delete[] grid;
    
    // Getting width and height
    char emptyChar;
    fin >> width >> emptyChar >> height;

    // Create new grid
    grid = new IndexedCell[width*height];

    // Reading all other lines with grid data
    int y=0;
    std::string line;
    // Skipping extra line
    std::getline(fin, line);
    while (std::getline(fin, line)) {
        // Read all need charachters or less, if haven't
        for (int x=0; x < std::min(width, (unsigned)line.length()); ++x) {
            getCell({x, y}).loadFrom(line[x]);
        }
        // Check, if read all lines
        if (++y == height) {
            break;
        }
    }
}
