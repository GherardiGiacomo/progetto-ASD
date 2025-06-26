// TO DO, vedi indicazioni in industry.h
#include "industry.h"
#include <map>       // Per std::map
#include <set>       // Per std::set
#include <climits>   // Per UINT_MAX

namespace industry
{

  struct Item
  {
    std::string name;
    bool isBasic;
    unsigned quantity;       // Usato solo per basic items
    list::List dependencies; // Items necessari per produrre questo item (A -> B, B è in dependencies di A)
    list::List dependents;   // Items che richiedono questo item (B -> A, B è in dependents di A)
  };

  struct st_Industry
  {
    Item *items;
    int size;
    int maxsize;
  };

  // Funzione ausiliaria per trovare l'indice di un item
  int findItemIndex(const Industry &indus, const std::string &name)
  {
    for (int i = 0; i < indus->size; ++i)
    {
      if (indus->items[i].name == name)
        return i;
    }
    return -1;
  }

  // Funzione ausiliaria per ordinare una lista lessicograficamente
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

  // Funzione ricorsiva per la rilevazione di cicli (DFS)
  // Controlla se 'startNode' può raggiungere 'endNode' seguendo le dipendenze
  bool hasPath(const Industry &indus, const std::string &startNode, const std::string &endNode, std::set<std::string> &visited)
  {
    if (startNode == endNode)
    {
      return true;
    }
    if (visited.count(startNode))
    {
      return false; // Già visitato in questo percorso, nessun ciclo
    }

    visited.insert(startNode);

    int idx = findItemIndex(indus, startNode);
    if (idx == -1)
    {
      return false; // Nodo non trovato (dovrebbe già essere stato gestito dall'esterno)
    }

    // Segui le dipendenze (componenti necessarie per produrre startNode)
    list::List dependencies = indus->items[idx].dependencies;
    for (int i = 0; i < list::size(dependencies); ++i)
    {
      std::string depName = list::get(i, dependencies);
      if (hasPath(indus, depName, endNode, visited))
      {
        return true;
      }
    }
    return false;
  }

  // Funzione ricorsiva per raccogliere i "neededBy" (diretti o indiretti)
  void collectNeededByChainRec(const Industry &indus, const std::string &name, std::set<std::string> &resultSet, std::set<std::string> &visited)
  {
    if (visited.count(name))
    {
      return;
    }
    visited.insert(name);

    int idx = findItemIndex(indus, name);
    if (idx == -1)
    {
      return;
    }

    list::List dependents = indus->items[idx].dependents; // Questi sono gli item che dipendono da 'name'
    for (int i = 0; i < list::size(dependents); ++i)
    {
      std::string dependentName = list::get(i, dependents);
      if (resultSet.find(dependentName) == resultSet.end())
      { // Evita di aggiungere duplicati
        resultSet.insert(dependentName);
      }
      collectNeededByChainRec(indus, dependentName, resultSet, visited);
    }
  }

  // Implementazione delle funzioni richieste

  Industry createEmptyIndustry()
  {
    Industry ind = new st_Industry;
    ind->size = 0;
    ind->maxsize = 100; // Dimensione iniziale, può essere riallocato se necessario
    ind->items = new Item[ind->maxsize];
    return ind;
  }

  void destroyIndustry(Industry &indus)
  {
    if (indus == nullptr)
      return;
    for (int i = 0; i < indus->size; ++i)
    {
      list::clear(indus->items[i].dependencies);
      list::clear(indus->items[i].dependents);
    }
    delete[] indus->items;
    delete indus;
    indus = nullptr;
  }

  bool insertBasicItem(Industry &indus, std::string name)
  {
    if (findItemIndex(indus, name) != -1)
    {
      return false; // Item già esistente
    }

    if (indus->size == indus->maxsize)
    {
      // Riallocazione se la dimensione massima è stata raggiunta
      int newMaxsize = indus->maxsize * 2;
      Item *newItems = new Item[newMaxsize];
      for (int i = 0; i < indus->size; ++i)
      {
        newItems[i] = indus->items[i];
      }
      delete[] indus->items;
      indus->items = newItems;
      indus->maxsize = newMaxsize;
    }

    Item newItem;
    newItem.name = name;
    newItem.isBasic = true;
    newItem.quantity = 0; // Quantità iniziale per basic item
    newItem.dependencies = list::createEmpty();
    newItem.dependents = list::createEmpty();

    indus->items[indus->size++] = newItem;
    return true;
  }

  bool insertItem(Industry &indus, std::string name, std::string *components, size_t s)
  {
    if (findItemIndex(indus, name) != -1)
    {
      return false; // Item già esistente
    }
    if (s == 0 || components == nullptr)
    {
      return false; // Un item composto deve avere almeno una componente
    }

    // 1. Verifica che tutte le componenti esistano e che non siano vuote
    for (size_t i = 0; i < s; ++i)
    {
      if (findItemIndex(indus, components[i]) == -1)
      {
        return false; // Componente non esistente
      }
    }

    // 2. Verifica la formazione di cicli
    // Un ciclo si formerebbe se il nuovo item (o un item da cui dipende)
    // dipendesse a sua volta da 'name'
    for (size_t i = 0; i < s; ++i)
    {
      std::set<std::string> visited;
      if (hasPath(indus, components[i], name, visited))
      { // Se una componente può raggiungere il nuovo item, c'è un ciclo
        return false;
      }
    }

    if (indus->size == indus->maxsize)
    {
      // Riallocazione
      int newMaxsize = indus->maxsize * 2;
      Item *newItems = new Item[newMaxsize];
      for (int i = 0; i < indus->size; ++i)
      {
        newItems[i] = indus->items[i];
      }
      delete[] indus->items;
      indus->items = newItems;
      indus->maxsize = newMaxsize;
    }

    // 3. Inserimento dell'item e aggiornamento delle dipendenze
    Item newItem;
    newItem.name = name;
    newItem.isBasic = false;
    newItem.quantity = 0; // Non applicabile per item composti
    newItem.dependencies = list::createEmpty();
    newItem.dependents = list::createEmpty();

    for (size_t i = 0; i < s; ++i)
    {
      list::addBack(components[i], newItem.dependencies);
      int compIdx = findItemIndex(indus, components[i]);
      if (compIdx != -1)
      {
        list::addBack(name, indus->items[compIdx].dependents);
      }
    }

    indus->items[indus->size++] = newItem;
    return true;
  }

  bool isPresentItem(const Industry &indus, std::string name)
  {
    return findItemIndex(indus, name) != -1;
  }

  // Funzione ausiliaria per la rimozione ricorsiva
  // Raccoglie tutti gli item che devono essere rimossi a partire da 'nameToRemove'
  void collectItemsToRemove(const Industry &indus, const std::string &nameToRemove, std::set<std::string> &itemsToRemoveSet)
  {
    if (itemsToRemoveSet.count(nameToRemove))
    {
      return; // Già marcato per la rimozione
    }

    itemsToRemoveSet.insert(nameToRemove);
    int idx = findItemIndex(indus, nameToRemove);
    if (idx == -1)
    {
      return; // Item non trovato (dovrebbe essere sempre presente se chiamato ricorsivamente)
    }

    list::List dependents = indus->items[idx].dependents;
    for (int i = 0; i < list::size(dependents); ++i)
    {
      std::string dependentName = list::get(i, dependents);
      collectItemsToRemove(indus, dependentName, itemsToRemoveSet);
    }
  }

  bool removeItem(Industry &indus, std::string name)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
    {
      return false; // Item non esistente
    }

    std::set<std::string> itemsToRemoveSet;
    collectItemsToRemove(indus, name, itemsToRemoveSet);

    // Crea un nuovo array di items escludendo quelli da rimuovere
    Item *newItems = new Item[indus->maxsize];
    int newSize = 0;

    for (int i = 0; i < indus->size; ++i)
    {
      if (itemsToRemoveSet.find(indus->items[i].name) == itemsToRemoveSet.end())
      {
        // Copia solo gli item che non devono essere rimossi
        newItems[newSize++] = indus->items[i];
      }
      else
      {
        // Pulisci le liste di dipendenza dell'item rimosso prima di "eliminarlo logicamente"
        list::clear(indus->items[i].dependencies);
        list::clear(indus->items[i].dependents);
      }
    }

    // Aggiorna le dipendenze degli item rimasti, rimuovendo riferimenti a item eliminati
    for (int i = 0; i < newSize; ++i)
    {
      list::List currentDependencies = newItems[i].dependencies;
      list::List newDependencies = list::createEmpty();
      for (int j = 0; j < list::size(currentDependencies); ++j)
      {
        std::string depName = list::get(j, currentDependencies);
        if (itemsToRemoveSet.find(depName) == itemsToRemoveSet.end())
        {
          list::addBack(depName, newDependencies);
        }
      }
      list::clear(newItems[i].dependencies);
      newItems[i].dependencies = newDependencies;

      list::List currentDependents = newItems[i].dependents;
      list::List newDependents = list::createEmpty();
      for (int j = 0; j < list::size(currentDependents); ++j)
      {
        std::string depName = list::get(j, currentDependents);
        if (itemsToRemoveSet.find(depName) == itemsToRemoveSet.end())
        {
          list::addBack(depName, newDependents);
        }
      }
      list::clear(newItems[i].dependents);
      newItems[i].dependents = newDependents;
    }

    delete[] indus->items;
    indus->items = newItems;
    indus->size = newSize;

    return true;
  }

  bool addBasicItem(Industry &indus, std::string name, int quantity)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1 || !indus->items[idx].isBasic)
    {
      return false; // Item non esistente o non è un basic item
    }

    if (quantity < 0)
    {
      unsigned int abs_quantity = (unsigned int)(-quantity);
      if (indus->items[idx].quantity < abs_quantity)
      {
        indus->items[idx].quantity = 0; // Non può andare sotto zero
      }
      else
      {
        indus->items[idx].quantity -= abs_quantity; // quantity è negativo, quindi sottrae
      }
    }
    else
    {
      indus->items[idx].quantity += quantity;
    }
    return true;
  }

  bool listNeed(const Industry &indus, std::string name, list::List &lres)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
    {
      lres = list::createEmpty(); // Assicurati che lres sia vuota in caso di fallimento
      return false;
    }

    lres = list::createEmpty();
    list::List dependencies = indus->items[idx].dependencies; // Dipendenze dirette
    for (int i = 0; i < list::size(dependencies); ++i)
    {
      list::addBack(list::get(i, dependencies), lres);
    }
    sortListLexicographically(lres);
    return true;
  }

  bool listNeededBy(const Industry &indus, std::string name, list::List &lres)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
    {
      lres = list::createEmpty(); // Assicurati che lres sia vuota in caso di fallimento
      return false;
    }

    lres = list::createEmpty();
    list::List dependents = indus->items[idx].dependents; // Dipendenti diretti
    for (int i = 0; i < list::size(dependents); ++i)
    {
      list::addBack(list::get(i, dependents), lres);
    }
    sortListLexicographically(lres);
    return true;
  }

  bool listNeededByChain(const Industry &indus, std::string name, list::List &lres)
  {
    int idx = findItemIndex(indus, name);
    if (idx == -1)
    {
      lres = list::createEmpty();
      return false;
    }

    std::set<std::string> resultSet;
    std::set<std::string> visited; // Per tracciare i nodi visitati nella DFS

    // Inizia la ricorsione dai diretti dipendenti
    list::List initialDependents = indus->items[idx].dependents;
    for (int i = 0; i < list::size(initialDependents); ++i)
    {
      std::string dependentName = list::get(i, initialDependents);
      if (resultSet.find(dependentName) == resultSet.end())
      { // Evita di aggiungere duplicati all'inizio
        resultSet.insert(dependentName);
      }
      collectNeededByChainRec(indus, dependentName, resultSet, visited);
    }

    lres = list::createEmpty();
    // Correzione del warning C++11: usare un iteratore tradizionale
    for (std::set<std::string>::const_iterator it = resultSet.begin(); it != resultSet.end(); ++it)
    {
      list::addBack(*it, lres);
    }
    sortListLexicographically(lres);
    return true;
  }

  // Funzione ausiliaria ricorsiva per calcolare le quantità totali di basic items
  // necessarie per produrre *una singola unità* di 'itemName'.
  // Popola 'requiredBasicItems' con coppie (nome_basic_item, quantità_necessaria_per_unità_di_itemName).
  // Restituisce true se 'itemName' esiste e la raccolta ha successo, false altrimenti.
  // Questa funzione è marcata come static per essere visibile solo all'interno di questo file.
  static bool getRequiredBasicItems(const Industry &indus, const std::string &itemName, std::map<std::string, unsigned> &requiredBasicItems)
  {
    int idx = findItemIndex(indus, itemName);
    if (idx == -1)
    {
      return false; // Item non trovato
    }

    const Item &item = indus->items[idx];

    if (item.isBasic)
    {
      requiredBasicItems[item.name] += 1; // 1 unità del basic item 'item.name' è necessaria
      return true;
    }

    // Per un item composto, ricorsivamente calcola le necessità per le sue dipendenze
    for (int i = 0; i < list::size(item.dependencies); ++i)
    {
      std::string dependencyName = list::get(i, item.dependencies);
      // Non c'è bisogno di una mappa temporanea per ogni dipendenza,
      // basta passare la stessa 'requiredBasicItems' e far sommare la ricorsione.
      if (!getRequiredBasicItems(indus, dependencyName, requiredBasicItems))
      {
        // Se una dipendenza non esiste o non può essere risolta, allora 'itemName' non può essere prodotto.
        return false;
      }
    }
    return true;
  }

  // Funzione principale per howManyItem
  bool howManyItem(const Industry &indus, std::string name, unsigned int &res)
  {
    // Non è necessaria una cache per howManyItemMemoizationCache qui
    // perché il problema richiede una funzione howManyItem che calcola
    // il massimo producibile date le quantità attuali, non memoizzando
    // i risultati tra chiamate distinte se le quantità cambiano.
    // La cache nel codice originale era probabilmente per calculateMaxProducible
    // come una funzione ricorsiva interna, che è stata sostituita da getRequiredBasicItems.
    // Pertanto, howManyItemMemoizationCache può essere rimossa del tutto.

    int idx = findItemIndex(indus, name);
    if (idx == -1)
    {
      res = 0;
      return false; // Item non esistente
    }

    const Item &item = indus->items[idx];

    // Se è un basic item, il risultato è la sua quantità disponibile
    if (item.isBasic)
    {
      res = item.quantity;
      return true;
    }

    // Per item composti:
    // Calcola le quantità totali di ogni basic item necessarie per *una* unità di 'name'.
    std::map<std::string, unsigned> requiredBasicItemsQuantities;
    // Inizializza la mappa per assicurarsi che i valori siano 0 prima di sommare
    // (std::map default-costruisce gli elementi a 0 per tipi numerici quando si usa [])

    if (!getRequiredBasicItems(indus, name, requiredBasicItemsQuantities))
    {
      // Se non riusciamo a determinare i requisiti base (e.g., item non trovato nella catena di dipendenze),
      // allora l'item non è producibile.
      res = 0;
      return false; // Indicare un fallimento nel trovare i requisiti base.
    }

    unsigned int minProducible = UINT_MAX;

    if (requiredBasicItemsQuantities.empty())
    {
      // Un item composto che non richiede alcun basic item per essere prodotto.
      // Questo implica che non ha dipendenze che si risolvono in basic items.
      // Nella logica di produzione, questo significa che non può essere "costruito"
      // da risorse iniziali, quindi la sua producibilità è 0.
      res = 0;
      return true; // L'item esiste, ma la sua producibilità è 0.
    }

    // Itera su tutti i basic items richiesti e calcola il limite di produzione
    for (std::map<std::string, unsigned>::const_iterator it = requiredBasicItemsQuantities.begin(); it != requiredBasicItemsQuantities.end(); ++it)
    {
      std::string basicItemName = it->first;
      unsigned int quantityNeededPerUnit = it->second;

      // Questo caso non dovrebbe verificarsi se getRequiredBasicItems è implementato correttamente,
      // dato che inserisce solo quantità positive.
      if (quantityNeededPerUnit == 0)
      {
        continue;
      }

      int basicItemIdx = findItemIndex(indus, basicItemName);
      if (basicItemIdx == -1 || !indus->items[basicItemIdx].isBasic)
      {
        // Se un basic item richiesto non esiste o non è un basic item, allora la produzione è 0.
        // Questo potrebbe indicare un'inconsistenza nella struttura dell'industria.
        res = 0;
        return true; // L'item esiste, ma non è producibile a causa di un componente mancante.
      }

      unsigned int availableQuantity = indus->items[basicItemIdx].quantity;

      unsigned int currentProducible = availableQuantity / quantityNeededPerUnit;

      if (currentProducible < minProducible)
      {
        minProducible = currentProducible;
      }
    }

    res = minProducible;
    return true;
  }

  // La funzione calculateMaxProducible e la howManyItemCache precedentemente definite sono state sostituite
  // e la loro logica integrata direttamente in howManyItem e nella nuova funzione getRequiredBasicItems.
  // Se la funzione calculateMaxProducible non è chiamata da nessun'altra parte, può essere rimossa.
  // Nel contesto di questa revisione, viene considerata rimossa/sostituita.

}