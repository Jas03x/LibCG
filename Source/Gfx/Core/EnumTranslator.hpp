#ifndef CG_ENUM_TRANSLATOR_HPP
#define CG_ENUM_TRANSLATOR_HPP

#include "CgDef.hpp"
#include "CgGfx.hpp"

enum DXGI_FORMAT;
enum D3D12_COMMAND_LIST_TYPE;
enum D3D12_INPUT_CLASSIFICATION;

class EnumTranslator
{
public:
	static bool CommandBufferType_To_CommandListType(COMMAND_BUFFER_TYPE CmdBufType, D3D12_COMMAND_LIST_TYPE& rCmdListType);
	static bool CommandQueueType_To_CommandListType(COMMAND_QUEUE_TYPE CmdQueueType, D3D12_COMMAND_LIST_TYPE& rCmdListType);

	static bool InputElement_To_SemanticName(INPUT_ELEMENT Element, const char*& rSemanticName);
	static bool InputElementFormat_To_DxgiFormat(INPUT_ELEMENT_FORMAT ElementFormat, DXGI_FORMAT& rDxgiFormat);
	static bool	InputElementType_To_InputSlotClass(INPUT_ELEMENT_TYPE ElementType, D3D12_INPUT_CLASSIFICATION& rInputSlotClass);
};

#endif // CG_ENUM_TRANSLATOR_HPP