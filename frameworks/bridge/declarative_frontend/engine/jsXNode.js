/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
var DirtyFlag;
(function (DirtyFlag) {
    DirtyFlag[DirtyFlag["UPDATE_PROPERTY"] = 0] = "UPDATE_PROPERTY";
    DirtyFlag[DirtyFlag["UPDATE_CONTENT"] = 1] = "UPDATE_CONTENT";
})(DirtyFlag || (DirtyFlag = {}));
class BaseNode extends __JSBaseNode__ {
}
/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/// <reference path="../../state_mgmt/src/lib/common/ifelse_native.d.ts" />
/// <reference path="./index.d.ts" />
/// <reference path="./base_node.ts" />
class BuilderNode extends BaseNode {
    constructor(uiContext) {
        super();
        var instanceId = -1;
        if (uiContext === undefined) {
            throw Error("BuilderNode construtor error, parem uicontext error");
        }
        else {
            if (!(typeof uiContext === "object") || !("instanceId_" in uiContext)) {
                throw Error("BuilderNode construtor error, parem uicontext is invalid");
            }
            instanceId = uiContext.instanceId_;
        }
        this.instanceId_ = instanceId;
        this.updateFuncByElmtId = new Map();
    }
    getCardId() {
        return -1;
    }
    addChild(child) {
        return false;
    }
    build(builder, params) {
        __JSScopeUtil__.syncInstanceId(this.instanceId_);
        this.params = params;
        this.updateFuncByElmtId.clear();
        super.build(builder.builder, this.params);
        __JSScopeUtil__.restoreInstanceId();
    }
    observeComponentCreation(func) {
        var elmId = ViewStackProcessor.AllocateNewElmetIdForNextComponent();
        func(elmId, true);
    }
    observeComponentCreation2(compilerAssignedUpdateFunc, classObject) {
        const _componentName = (classObject && ("name" in classObject)) ? Reflect.get(classObject, "name") : "unspecified UINode";
        const _popFunc = (classObject && "pop" in classObject) ? classObject.pop : () => { };
        const updateFunc = (elmtId, isFirstRender, instanceId = this.instanceId_) => {
            __JSScopeUtil__.syncInstanceId(instanceId);
            ViewStackProcessor.StartGetAccessRecordingFor(elmtId);
            compilerAssignedUpdateFunc(elmtId, isFirstRender);
            if (!isFirstRender) {
                _popFunc();
            }
            ViewStackProcessor.StopGetAccessRecording();
            __JSScopeUtil__.restoreInstanceId();
        };
        const elmtId = ViewStackProcessor.AllocateNewElmetIdForNextComponent();
        // needs to move set before updateFunc.
        // make sure the key and object value exist since it will add node in attributeModifier during updateFunc.
        this.updateFuncByElmtId.set(elmtId, { updateFunc: updateFunc, componentName: _componentName });
        try {
            updateFunc(elmtId, /* is first render */ true);
        }
        catch (error) {
            // avoid the incompatible change that move set function before updateFunc.
            this.updateFuncByElmtId.delete(elmtId);
            throw error;
        }
    }
    /**
     Partial updates for ForEach.
     * @param elmtId ID of element.
     * @param itemArray Array of items for use of itemGenFunc.
     * @param itemGenFunc Item generation function to generate new elements. If index parameter is
     *                    given set itemGenFuncUsesIndex to true.
     * @param idGenFunc   ID generation function to generate unique ID for each element. If index parameter is
     *                    given set idGenFuncUsesIndex to true.
     * @param itemGenFuncUsesIndex itemGenFunc optional index parameter is given or not.
     * @param idGenFuncUsesIndex idGenFunc optional index parameter is given or not.
     */
    forEachUpdateFunction(elmtId, itemArray, itemGenFunc, idGenFunc, itemGenFuncUsesIndex = false, idGenFuncUsesIndex = false) {
        if (itemArray === null || itemArray === undefined) {
            return;
        }
        if (itemGenFunc === null || itemGenFunc === undefined) {
            return;
        }
        if (idGenFunc === undefined) {
            idGenFuncUsesIndex = true;
            // catch possible error caused by Stringify and re-throw an Error with a meaningful (!) error message
            idGenFunc = (item, index) => {
                try {
                    return `${index}__${JSON.stringify(item)}`;
                }
                catch (e) {
                    throw new Error(` ForEach id ${elmtId}: use of default id generator function not possible on provided data structure. Need to specify id generator function (ForEach 3rd parameter). Application Error!`);
                }
            };
        }
        let diffIndexArray = []; // New indexes compared to old one.
        let newIdArray = [];
        let idDuplicates = [];
        const arr = itemArray; // just to trigger a 'get' onto the array
        // ID gen is with index.
        if (idGenFuncUsesIndex) {
            // Create array of new ids.
            arr.forEach((item, indx) => {
                newIdArray.push(idGenFunc(item, indx));
            });
        }
        else {
            // Create array of new ids.
            arr.forEach((item, index) => {
                newIdArray.push(`${itemGenFuncUsesIndex ? index + '_' : ''}` + idGenFunc(item));
            });
        }
        // Set new array on C++ side.
        // C++ returns array of indexes of newly added array items.
        // these are indexes in new child list.
        ForEach.setIdArray(elmtId, newIdArray, diffIndexArray, idDuplicates);
        // Item gen is with index.
        diffIndexArray.forEach((indx) => {
            ForEach.createNewChildStart(newIdArray[indx], this);
            if (itemGenFuncUsesIndex) {
                itemGenFunc(arr[indx], indx);
            }
            else {
                itemGenFunc(arr[indx]);
            }
            ForEach.createNewChildFinish(newIdArray[indx], this);
        });
    }
    ifElseBranchUpdateFunction(branchId, branchfunc) {
        const oldBranchid = If.getBranchId();
        if (branchId == oldBranchid) {
            return;
        }
        // branchid identifies uniquely the if .. <1> .. else if .<2>. else .<3>.branch
        // ifElseNode stores the most recent branch, so we can compare
        // removedChildElmtIds will be filled with the elmtIds of all childten and their children will be deleted in response to if .. else chnage
        var removedChildElmtIds = new Array();
        If.branchId(branchId, removedChildElmtIds);
        branchfunc();
    }
}
/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/// <reference path="./builder_node.ts" />
const arkUINativeModule = globalThis.getArkUINativeModule();
function GetUINativeModule() {
    if (arkUINativeModule) {
        return arkUINativeModule;
    }
    return arkUINativeModule;
}
class NodeController {
    constructor() {
        this.nodeContainerId_ = -1;
    }
    aboutToResize(size) { }
    aboutToAppear() { }
    aboutToDisappear() { }
    rebuild() {
        if (this.nodeContainerId_ >= 0) {
            GetUINativeModule().nodeContainer.rebuild(this.nodeContainerId_);
        }
    }
}

export default { NodeController, BuilderNode, BaseNode, DirtyFlag };
