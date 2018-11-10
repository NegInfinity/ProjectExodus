#include "JsonImportPrivatePCH.h"
#include "MaterialBuilder.h"

void MaterialBuilder::arrangeNodesGrid(UMaterial* material, const JsonMaterial &jsonMat, 
		const MaterialFingerprint &fingerprint, MaterialBuildData &buildData){
	auto numExpressions = material->Expressions.Num();
	int expressionRows = (int)(sqrtf((float)numExpressions))+1;
	if (expressionRows == 0)
		return;

	for (int i = 0; i < numExpressions; i++){
		auto cur = material->Expressions[i];
		auto row = i / expressionRows;
		auto col = i % expressionRows;

		int32 size = 256;

		int32 x = (col - expressionRows) * size;
		int32 y = row * size;

		cur->MaterialExpressionEditorX = x;
		cur->MaterialExpressionEditorY = y;
	}
}

void MaterialBuilder::arrangeNodesTree(UMaterial* material /*, const JsonMaterial &jsonMat, const MaterialFingerprint &fingerprint, MaterialBuildData &buildData*/){
	TMap<UMaterialExpression*, TSet<UMaterialExpression*>> srcToDst, dstToSrc;
	auto registerConnection = [&](UMaterialExpression* src, UMaterialExpression* dst){
		if (!src || !dst)
			return;
		if (src == dst)
			return;
		srcToDst.FindOrAdd(src).Add(dst);
		dstToSrc.FindOrAdd(dst).Add(src);
	};

	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Sorting expressions"));
	//calculate connection chain
	for(auto curExpr: material->Expressions){
		auto inputs = curExpr->GetInputs();
		for(auto curInput: inputs){
			if (!curInput || !curInput->Expression)
				continue;
			registerConnection(curInput->Expression, curExpr);
		}
	}
	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("srcToDst connections: %d, dstToSrc connections: %d"), srcToDst.Num(), dstToSrc.Num());

	TSet<UMaterialExpression*> topLevel;
	//find top level ones
	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Gathering top level nodes"));
	for(auto curExpr: material->Expressions){
		if (!srcToDst.Contains(curExpr))
			topLevel.Add(curExpr);
	}
	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("%d nodes found"), topLevel.Num());

	TMap<UMaterialExpression*, int> exprLevels;
	TQueue<UMaterialExpression*> unprocessed;
	TMap<int, TArray<UMaterialExpression*>> levelOrder;

	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Enqueueing top-level nodes"));
	for(auto curExpr: topLevel){
		unprocessed.Enqueue(curExpr);
	}

	int defaultParamLevel = 1024;
	int maxLevel = defaultParamLevel;//0;
	while(!unprocessed.IsEmpty()){
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Processing items still in queue"));
		UMaterialExpression* curExpr = 0;
		if (!unprocessed.Dequeue(curExpr))
			break;
		if (!curExpr)
			continue;
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Current node: %x, %s"), curExpr, *curExpr->GetName());
		int curLevel = 0;
		if (!dstToSrc.Contains(curExpr))
			curLevel = defaultParamLevel;
		const auto foundLevel= exprLevels.Find(curExpr);
		if (foundLevel){
			curLevel = *foundLevel;
		}
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Current level: %d"), curLevel);

		auto& curOrder = levelOrder.FindOrAdd(curLevel);
		curOrder.Add(curExpr);
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("%d items at current level"), curOrder.Num());

		const auto children = dstToSrc.Find(curExpr);
		if (!children)
			continue;
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("%d children found"), children->Num());
		for(auto child: *children){
			if (!child)
				continue;

			auto childLevel = curLevel + 1;
			if (!dstToSrc.Contains(child))
				childLevel = defaultParamLevel;
			//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Processing child %s (%x)"), *child->GetName(), child);
			auto prevLevel = exprLevels.Find(child);
			if (prevLevel){
				if (*prevLevel >= childLevel){
					//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Child %s (%x) already registered on level %d"), *child->GetName(), child, *prevLevel);
					continue;
				}
				levelOrder.FindOrAdd(*prevLevel).Remove(child);
			}
			//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Child %s (%x) enqueued for processing"), *child->GetName(), child);
			unprocessed.Enqueue(child);
			exprLevels.Add(child, childLevel);
			if (childLevel > maxLevel)
				maxLevel = childLevel;
		}
	}

	TArray<int> usedLevels;
	auto maxNumItems = 0;
	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Gathering used levels"));
	for(int i = 0; i <= maxLevel; i++){
		auto numItems = levelOrder.FindOrAdd(i).Num();
		if (numItems > 0)
			usedLevels.Add(i);
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Registered level %d"), i);
		if (numItems > maxNumItems)
			maxNumItems = numItems;
	}
	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("%d levels total"), usedLevels.Num());

	//Now, actual grid.

	auto numRows = maxNumItems;
	auto numColumns = usedLevels.Num();
	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("numRows: %d; numColumns: %d"), numRows, numColumns);
	if ((numRows == 0) || (numColumns == 0)){
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Nothing to do, returning"));
		return;
	}


#if 0
	int32 xSize = 128;
	int32 ySize = 256;
	//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Sorting material node tree"));
	for(int col = 0; col < usedLevels.Num(); col++){
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Processing column %d out of %d"), col, usedLevels.Num());
		int levelIndex = usedLevels[col];
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Current level %d"), levelIndex);
		int x = xSize * (-1 -col);//(col - numColumns);
		const auto &curLevel = levelOrder.FindOrAdd(levelIndex);
		int curNumRows = curLevel.Num();
		int yOffset = (maxNumItems - curNumRows) * ySize / 2;
		//UE_LOG(JsonLogMatNodeSort, Log, TEXT("Num rows: %d, yOffset: %d"), curNumRows, yOffset);
		for (int row = 0; row < curNumRows; row++){
			int y = yOffset + row * ySize;
			auto item = curLevel[row];
			if (!item)
				continue;
			item->MaterialExpressionEditorX = x;
			item->MaterialExpressionEditorY = y;
			//UE_LOG(JsonLogMatNodeSort, Log, TEXT("x: %d; y: %d for item %s(%x)"), x, y, *item->GetName(), item);
		}
	}
#endif
	int32 x = -256;
	const int32 xPadding = 32;
	const int32 yPadding = 32;
	const int32 yOutputSize = 8;
	const int32 yHeaderSize = 8;
	/*
	const int32 xItemSize = 128;
	const int32 yItemSize = 256;
	const int32 yMinSize = 32;
	*/

	auto getItemHeight = [&](UMaterialExpression *arg) -> auto{
		if (!arg)
			return 0;
		auto h = arg->GetHeight();
		auto numOuts = arg->Outputs.Num();
		auto outputSize = yHeaderSize + yOutputSize *numOuts;
		if (h < outputSize)
			h = outputSize;
		return h;
	};

	for(int col = 0; col < usedLevels.Num(); col++){
		int levelIndex = usedLevels[col];
		const auto &curLevel = levelOrder.FindOrAdd(levelIndex);
		int curNumRows = curLevel.Num();

		int xSize = 0;
		int ySize = 0;
		for (int row = 0; row < curNumRows; row++){
			auto item = curLevel[row];
			if (!item)
				continue;
			auto w = item->GetWidth();
			auto h = getItemHeight(item);//item->GetHeight();
			if (w > xSize)
				xSize = w;

			ySize += h;
			if (row > 0)
				ySize += yPadding;
		}

		//int ySize = yItemSize * curNumRows;
		int yOffset = -ySize/2;
		int y = yOffset;
		x -= xSize;
		for (int row = 0; row < curNumRows; row++){
			//int y = yOffset + row * ySize;
			auto item = curLevel[row];
			if (!item)
				continue;
			auto w = item->GetWidth();
			auto h = getItemHeight(item);//item->GetHeight();
			item->MaterialExpressionEditorX = x;
			item->MaterialExpressionEditorY = y;
			y += h;
			if (row)
				y += yPadding;
		}
		x -= xPadding;
	}
}

