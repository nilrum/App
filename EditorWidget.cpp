//
// Created by user on 11.01.2021.
//

#include "EditorWidget.h"

INIT_PROPERTYS(TEditorWidget)

TEditorWidget::TEditorWidget()
{
    name = "EditorWidget";
    title = "Editor";
}

bool TEditorWidget::IsButtons() const
{
    return isButtons;
}

void TEditorWidget::SetIsButtons(bool value)
{
    isButtons = value;
}
