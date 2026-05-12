#ifdef DrawText
#undef DrawText
#endif
#include "ShaderConditionalFrame.hpp"

bool ShaderConditionalFrame::get_BranchMatched()
{
return this->BranchMatched;
}

void ShaderConditionalFrame::set_BranchMatched(bool value)
{
this->BranchMatched = value;
}

bool ShaderConditionalFrame::get_CurrentIncluded()
{
return this->CurrentIncluded;
}

void ShaderConditionalFrame::set_CurrentIncluded(bool value)
{
this->CurrentIncluded = value;
}

bool ShaderConditionalFrame::get_HasElseBranch()
{
return this->HasElseBranch;
}

void ShaderConditionalFrame::set_HasElseBranch(bool value)
{
this->HasElseBranch = value;
}

bool ShaderConditionalFrame::get_ParentIncluded()
{
return this->ParentIncluded;
}

ShaderConditionalFrame::ShaderConditionalFrame(bool parentIncluded, bool branchMatched, bool currentIncluded) : BranchMatched(), CurrentIncluded(), HasElseBranch(), ParentIncluded()
{
this->ParentIncluded = parentIncluded;
this->set_BranchMatched(branchMatched);
this->set_CurrentIncluded(currentIncluded);
}

