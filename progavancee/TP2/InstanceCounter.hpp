#ifndef INSTANCE_COUNTER_HPP  // Garde contre les inclusions multiples
#define INSTANCE_COUNTER_HPP

template <typename T>
class CompteurInstances {
    public:
        static int compteurCreation;
        static int compteurDestruction;

        CompteurInstances() { ++compteurCreation; }
        ~CompteurInstances() { ++compteurDestruction; }

        static int instancesExistantes() { 
            return compteurCreation - compteurDestruction; 
        }
        static int totalCreations() { return compteurCreation; }
        static int totalDestructions() { return compteurDestruction; }
};

// Initialisation des variables statiques (dans le header pour les templates)
template <typename T>
int CompteurInstances<T>::compteurCreation = 0;

template <typename T>
int CompteurInstances<T>::compteurDestruction = 0;

#endif // INSTANCE_COUNTER_HPP