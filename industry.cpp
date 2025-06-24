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

  // Inserisce un nuovo item composto
  bool insertItem(Industry &indus, std::string name, std::string* components, size_t s)
  {
    if (isPresentItem(indus, name))
      return false;

    // Verifica che tutti i componenti esistano
    for (size_t i = 0; i < s; ++i)
    {
      if (!isPresentItem(indus, components[i]))
        return false;
    }

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

    // Crea il nuovo item composto
    Item newItem;
    newItem.name = name;
    newItem.isBasic = false;
    newItem.quantity = 0; // non utilizzato per item composti
    newItem.dependencies = list::createEmpty();
    newItem.dependents = list::createEmpty();

    // Aggiungi le dipendenze
    for (size_t i = 0; i < s; ++i)
    {
      list::addBack(components[i], newItem.dependencies);
      
      // Aggiungi questo item alla lista dei dependents del componente
      int compIdx = findItemIndex(indus, components[i]);
      if (compIdx != -1)
      {
        list::addBack(name, indus->items[compIdx].dependents);
      }
    }

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
      lres = list::createEmpty();
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

  // Restituisce gli item che dipendono direttamente da name
  bool listNeededBy(const Industry &indus, std::string name, list::List &lres)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
    {
      lres = list::createEmpty();
      return false;
    }

    // Copia la lista dei dependents diretti
    lres = list::createEmpty();
    list::List &deps = indus->items[idx].dependents;
    for (int i = 0; i < list::size(deps); ++i)
    {
      list::addBack(list::get(i, deps), lres);
    }

    // Ordina alfabeticamente
    sortListLexicographically(lres);

    return true;
  }

  // Restituisce tutti gli item che dipendono (direttamente o indirettamente) da name
  bool listNeededByChain(const Industry &indus, std::string name, list::List &lres)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
    {
      lres = list::createEmpty();
      return false;
    }

    // Usa la funzione ausiliaria già esistente per raccogliere tutti i dependents
    lres = list::createEmpty();
    collectDependentsRec(indus, name, lres);

    // Ordina alfabeticamente
    sortListLexicographically(lres);

    return true;
  }

  // Funzione ausiliaria per calcolare ricorsivamente quanti item si possono produrre
  unsigned calculateMaxProducible(const Industry &indus, const std::string &name)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
      return 0;

    const Item &item = indus->items[idx];

    // Se è un basic item, ritorna la quantità disponibile
    if (item.isBasic)
    {
      return item.quantity;
    }

    // Se è un item composto, calcola il minimo tra tutti i componenti
    unsigned minProducible = UINT_MAX;
    bool hasComponents = false;

    for (int i = 0; i < list::size(item.dependencies); ++i)
    {
      hasComponents = true;
      std::string componentName = list::get(i, item.dependencies);
      unsigned componentMax = calculateMaxProducible(indus, componentName);
      
      if (componentMax < minProducible)
      {
        minProducible = componentMax;
      }
    }

    // Se non ha componenti (caso anomalo), ritorna 0
    if (!hasComponents)
      return 0;

    return minProducible;
  }

  // Calcola quante unità di un item si possono produrre
  bool howManyItem(const Industry &indus, std::string name, unsigned &res)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
    {
      res = 0;
      return false;
    }

    res = calculateMaxProducible(indus, name);
    return true;
  }

} // fine del namespace industry