cmake_minimum_required(VERSION 2.8)

project(doc)

LIST(APPEND doc_files lmalpha.tex
    lmalpha_design.tex
    lmalpha_practice.tex
    lmalpha_usecase.tex)

add_custom_target(doc ALL SOURCES  ${doc_files})

