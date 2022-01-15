#!/bin/bash

#creation de variables

repBase=$1
repArrive=$2
resolution=$3

#fonction qui va tester si on a au moins deux arguments

if [ "$#" -lt 2 ] ; then    #lt signifie "less than", c'est a dire si on a moins de deux arguments
    echo "Pas assez d'arguments en entree" #on envoie un message sur le terminal
    exit
fi 



#fonction qui cree un repertoire d'arrivee au cas ou ce dernier n'existe pas

function test_arrive() {
    if [ -d $repArrive ] ; then #Si le dossier existe
        echo "Le dossier existe"
    else
        mkdir -p $repArrive #on cree le dossier d'arrive
    fi
}

#fonction qui va supprimer  les espaces, les apostrophes et guillemets des noms de fichier dans l'image d'arrivee

function sup_arrivee() {
    mv $1 $repBase/$(echo $(basename $1) | sed -r "s/[[:space:]\'!]+//g") #enleve les espaces, apostrophes, guillemets
}

#fonction pour que l'image d'arrivee soit en PNG

function arr_png() {
    if [[ $(file -i $1 | awk '{ print $2 }') = image/* ]] ; then #test si le fichier est une image et si le deuxieme argument du type mim est image/*
        if [ -z "$resolution" ] ; then #test si on a une resolution
            convert $1 $repBase/$(basename ${1%.*}).png #convertit l'image
        else
            convert -resize $resolution $1 $repBase/$(basename ${1%.*}).png #modifie la taille de l'image et convertit
        fi
    else
        echo "'$1' n'est pas une image" 
    fi
}


test_arrive #on appelle la fonction de sorte a ce qu'elle agisse

for fichier in "$repBase"/*; do #boucle qui pour tout fichier du repertoire demande copie et converti toutes les images
    sup_arrivee "$fichier"
    arr_png "$fichier"
    if [[ $fichier == *.png ]] ; then
        cp $fichier $repArrive 
    else
        echo "'$fichier' ne peux pas etre copie"
    fi
done





