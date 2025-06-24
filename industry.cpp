// TO DO, vedi indicazioni in industry.h
#include "industry.h"

namespace industry
{

  // Struttura interna per rappresentare un singolo item
  struct Item
  {
    std::string name;
    bool isBasic;
    unsigned quantity;       // valido solo se isBasic == true
    list::List dependencies; // nomi degli item da cui dipende (in ingresso)
    list::List dependents;   // nomi degli item che dipendono da questo (in uscita)
  };

  // Struttura principale dell'industria
  struct st_Industry
  {
    Item *items;
    int size;
    int maxsize;
  };

  int findItemIndex(const Industry &indus, const std::string &name)
  {
    for (int i = 0; i < indus->size; ++i)
    {
      if (indus->items[i].name == name)
      {
        return i;
      }
    }
    return -1; // non trovato
  }
  void sortListLexicographically(list::List &l)
  {
    int n = list::size(l);
    for (int i = 0; i < n - 1; ++i)
    {
      for (int j = i + 1; j < n; ++j)
      {
        if (list::get(j, l) < list::get(i, l))
        {
          std::string tmp = list::get(i, l);
          list::set(i, list::get(j, l), l);
          list::set(j, tmp, l);
        }
      }
    }
  }

  // Crea una nuova industria vuota
  Industry createEmptyIndustry()
  {
    Industry ind = new st_Industry;
    ind->size = 0;
    ind->maxsize = 100;
    ind->items = new Item[ind->maxsize];
    return ind;
  }

  // Verifica se esiste un item con il nome specificato
  bool isPresentItem(const Industry &indus, std::string name)
  {
    for (int i = 0; i < indus->size; ++i)
    {
      if (indus->items[i].name == name)
        return true;
    }
    return false;
  }

  // Inserisce un nuovo basic item
  bool insertBasicItem(Industry &indus, std::string name)
  {
    if (isPresentItem(indus, name))
      return false;

    // Rialloca memoria se necessario
    if (indus->size == indus->maxsize)
    {
      int newCap = indus->maxsize + 100;
      Item *newItems = new Item[newCap];
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

  void collectDependentsRec(const Industry &indus, const std::string &name, list::List &result)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
      return;

    for (int i = 0; i < list::size(indus->items[idx].dependents); ++i)
    {
      std::string depName = list::get(i, indus->items[idx].dependents);

      // Se depName non è già nella lista, aggiungilo e visita ricorsivamente
      bool alreadyIn = false;
      for (int j = 0; j < list::size(result); ++j)
      {
        if (list::get(j, result) == depName)
        {
          alreadyIn = true;
          break;
        }
      }

      if (!alreadyIn)
      {
        list::addBack(depName, result);
        collectDependentsRec(indus, depName, result);
      }
    }
  }

  bool removeItem(Industry &indus, std::string name)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
      return false;

    // Lista di nomi da rimuovere (compreso il nodo di partenza)
    list::List toRemove = list::createEmpty();
    list::addBack(name, toRemove);
    collectDependentsRec(indus, name, toRemove);

    // Rimuovi ogni item della lista
    for (int i = 0; i < list::size(toRemove); ++i)
    {
      std::string currentName = list::get(i, toRemove);

      int index = findItemIndex(indus, currentName);
      if (index == -1)
        continue;

      // Prima di rimuoverlo, elimina il suo nome dalle dipendenze/dependenti degli altri
      for (int j = 0; j < indus->size; ++j)
      {
        if (j == index)
          continue;

        // Rimuovi currentName da dependencies
        list::List &deps = indus->items[j].dependencies;
        for (int k = 0; k < list::size(deps); ++k)
        {
          if (list::get(k, deps) == currentName)
          {
            list::removePos(k, deps);
            break;
          }
        }

        // Rimuovi currentName da dependents
        list::List &depsOf = indus->items[j].dependents;
        for (int k = 0; k < list::size(depsOf); ++k)
        {
          if (list::get(k, depsOf) == currentName)
          {
            list::removePos(k, depsOf);
            break;
          }
        }
      }

      // Rimuovi fisicamente l'item da indus->items
      for (int j = index; j < indus->size - 1; ++j)
      {
        indus->items[j] = indus->items[j + 1];
      }
      indus->size--;
    }

    return true;
  }
  bool addBasicItem(Industry &indus, std::string name, int v)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
      return false; // item non esiste

    Item &item = indus->items[idx];

    if (!item.isBasic)
      return false; // solo basic item possono avere quantità

    // Aggiunge o sottrae la quantità (mai sotto zero)
    int newQ = static_cast<int>(item.quantity) + v;
    if (newQ < 0)
      item.quantity = 0;
    else
      item.quantity = static_cast<unsigned>(newQ);

    return true;
  }
  bool listNeed(const Industry &indus, std::string name, list::List &lres)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
    {
      lres = list::createEmpty(); // oppure imposta a nullptr, ma createEmpty va bene
      return false;
    }

    // Copia la lista delle dipendenze dirette
    lres = list::createEmpty();
    list::List &deps = indus->items[idx].dependencies;
    for (int i = 0; i < list::size(deps); ++i)
    {
      list::addBack(list::get(i, deps), lres);
    }

    // Ordina alfabeticamente
    sortListLexicographically(lres);

    return true;
  }

} // fine del namespace industry
