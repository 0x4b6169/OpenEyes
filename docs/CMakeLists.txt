set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_HOME_DIRECTORY}/README.md")
set(DOXYGEN_GENERATE_TREEVIEW YES)

doxygen_add_docs(
        doxygen
        ${CMAKE_HOME_DIRECTORY}/README.md
        ${CMAKE_HOME_DIRECTORY}/core
        ${CMAKE_HOME_DIRECTORY}/docs
        ALL
        COMMENT "Generate html pages"
)
