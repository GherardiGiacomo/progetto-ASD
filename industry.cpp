// TO DO, vedi indicazioni in industry.h
#include "industry.h"

namespace industry {

  // Struttura interna per rappresentare un singolo item
  struct Item {
    std::string name;
    bool isBasic;
    unsigned quantity;              // valido solo se isBasic == true
    list::List dependencies;        // nomi degli item da cui dipende (in ingresso)
    list::List dependents;          // nomi degli item che dipendono da questo (in uscita)
  };

  // Struttura principale dell'industria
  struct st_Industry {
    Item* items;
    int size;
    int maxsize;
  };

  // Crea una nuova industria vuota
  Industry createEmptyIndustry() {
    Industry ind = new st_Industry;
    ind->size = 0;
    ind->maxsize = 100;
    ind->items = new Item[ind->maxsize];
    return ind;
  }

  // Verifica se esiste un item con il nome specificato
  bool isPresentItem(const Industry& indus, std::string name) {
    for (int i = 0; i < indus->size; ++i) {
      if (indus->items[i].name == name)
        return true;
    }
    return false;
  }

  // Inserisce un nuovo basic item
  bool insertBasicItem(Industry& indus, std::string name) {
    if (isPresentItem(indus, name))
      return false;

    // Rialloca memoria se necessario
    if (indus->size == indus->maxsize) {
      int newCap = indus->maxsize + 100;
      Item* newItems = new Item[newCap];
      for (int i = 0; i < indus->size; ++i)
        newItems[i] = indus->items[i];
      delete[] indus->items;
      indus->items = newItems;
      indus->maxsize = newCap;
    }

    // Crea e inserisce il nuovo item
    Item newItem;
    newItem.name = name;
    newItem.isBasic = true;
    newItem.quantity = 0;
    newItem.dependencies = list::createEmpty();
    newItem.dependents = list::createEmpty();

    indus->items[indus->size++] = newItem;
    return true;
  }

} // fine del namespace industry
